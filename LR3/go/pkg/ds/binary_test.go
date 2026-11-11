package ds

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"os"
	"path/filepath"
	"strconv"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

const cppDumpHex = "445344420141525259010000000000000005000000464c53540000000000" +
	"0000004c4c535400000000000000005354434b464c535400000000000000" +
	"00515545554c4c5354000000000000000043425452000000000000000048" +
	"41534801010000000000000005000000000000006170706c6503000000"

func cppDump(t *testing.T) []byte {
	t.Helper()
	data, err := hex.DecodeString(cppDumpHex)
	require.NoError(t, err)
	return data
}

func TestBinaryMatchesCppFormat(t *testing.T) {
	database := NewDatabase()
	database.Execute("MPUSH 5")
	database.Execute("HPUT apple 3")
	data, err := database.MarshalBinary()
	require.NoError(t, err)
	assert.Equal(t, cppDump(t), data)
}

func TestBinaryReadsFileWrittenByCpp(t *testing.T) {
	database := NewDatabase()
	require.NoError(t, database.UnmarshalBinary(cppDump(t)))
	assert.Equal(t, "5", database.Execute("PRINT M"))
	assert.Equal(t, "3", database.Execute("HGET apple"))
}

func TestBinaryRoundTrip(t *testing.T) {
	source := NewDatabase()
	queries := []string{
		"MPUSH 1", "MPUSH -2", "FPUSH 3", "FPUSH_END 4", "LPUSH 5", "LPUSH_END 6", "SPUSH 7", "SPUSH 8",
		"QPUSH 9", "QPUSH 10", "TINSERT 11", "TINSERT 12", "TINSERT 13", "HPUT key 14", "HPUT other -15",
	}
	for _, query := range queries {
		source.Execute(query)
	}
	data, err := source.MarshalBinary()
	require.NoError(t, err)

	loaded := NewDatabase()
	loaded.Execute("MPUSH 99")
	require.NoError(t, loaded.UnmarshalBinary(data))
	for _, query := range []string{"PRINT M", "PRINT F", "PRINT L", "PRINT S", "PRINT Q", "PRINT T", "PRINT H"} {
		assert.Equal(t, source.Execute(query), loaded.Execute(query), query)
	}
	assert.Equal(t, "8", loaded.Execute("SPOP"))
	assert.Equal(t, "9", loaded.Execute("QPOP"))
}

func TestBinaryRejectsTruncatedData(t *testing.T) {
	data := cppDump(t)
	for size := 0; size < len(data); size++ {
		database := NewDatabase()
		assert.ErrorIs(t, database.UnmarshalBinary(data[:size]), ErrFormat, "size=%d", size)
	}
}

func TestBinaryRejectsBadHeader(t *testing.T) {
	database := NewDatabase()
	assert.ErrorIs(t, database.UnmarshalBinary([]byte("XXXX\x01")), ErrFormat)
	assert.ErrorIs(t, database.UnmarshalBinary([]byte("DSDB\x63")), ErrFormat)

	data := cppDump(t)
	data[5] = 'X'
	assert.ErrorIs(t, database.UnmarshalBinary(data), ErrFormat)
}

func TestBinaryRejectsUnknownStrategy(t *testing.T) {
	data := cppDump(t)
	data[bytes.Index(data, []byte("HASH"))+4] = 9
	assert.ErrorIs(t, NewDatabase().UnmarshalBinary(data), ErrFormat)
}

func TestBinaryRejectsOversizedCounts(t *testing.T) {
	huge := binary.LittleEndian.AppendUint64(nil, 1<<40)
	tooLarge := append([]byte("DSDB\x01ARRY"), huge...)
	assert.ErrorIs(t, NewDatabase().UnmarshalBinary(tooLarge), ErrFormat)

	five := binary.LittleEndian.AppendUint64(nil, 5)
	tooShort := append([]byte("DSDB\x01ARRY"), five...)
	assert.ErrorIs(t, NewDatabase().UnmarshalBinary(tooShort), ErrFormat)
}

func TestBinaryRejectsOversizedString(t *testing.T) {
	data := cppDump(t)
	crafted := bytes.Clone(data[:bytes.Index(data, []byte("HASH"))+5])
	crafted = binary.LittleEndian.AppendUint64(crafted, 1)
	crafted = binary.LittleEndian.AppendUint64(crafted, 1<<40)
	assert.ErrorIs(t, NewDatabase().UnmarshalBinary(crafted), ErrFormat)
}

func TestBinaryRejectsValuesOutsideInt32(t *testing.T) {
	if strconv.IntSize < 64 {
		t.Skip("int is narrower than 64 bits")
	}
	path := filepath.Join(t.TempDir(), "big.bin")

	database := NewDatabase()
	database.Execute("MPUSH 3000000000")
	_, err := database.MarshalBinary()
	assert.ErrorIs(t, err, ErrFormat)
	assert.ErrorIs(t, database.SaveBinaryFile(path), ErrFormat)

	database = NewDatabase()
	database.Execute("HPUT key 3000000000")
	_, err = database.MarshalBinary()
	assert.ErrorIs(t, err, ErrFormat)
}

func TestBinaryFileRoundTrip(t *testing.T) {
	path := filepath.Join(t.TempDir(), "db.bin")
	source := NewDatabase()
	source.Execute("MPUSH 10")
	require.NoError(t, source.SaveBinaryFile(path))

	loaded := NewDatabase()
	require.NoError(t, loaded.LoadBinaryFile(path))
	assert.Equal(t, "10", loaded.Execute("MGET 0"))
}

func TestBinaryFileErrors(t *testing.T) {
	database := NewDatabase()
	dir := t.TempDir()
	assert.Error(t, database.LoadBinaryFile(filepath.Join(dir, "missing.bin")))
	assert.Error(t, database.SaveBinaryFile(filepath.Join(dir, "no_such_dir", "db.bin")))

	broken := filepath.Join(dir, "broken.bin")
	require.NoError(t, os.WriteFile(broken, []byte("garbage"), 0o600))
	assert.ErrorIs(t, database.LoadBinaryFile(broken), ErrFormat)
}

func TestCLIUsesBinaryFormatForBinFiles(t *testing.T) {
	path := filepath.Join(t.TempDir(), "cli.bin")
	var out, errOut bytes.Buffer
	assert.Equal(t, 0, RunCLI([]string{"--file", path, "--query", "MPUSH 5"}, &out, &errOut))

	content, err := os.ReadFile(path)
	require.NoError(t, err)
	assert.Equal(t, "DSDB", string(content[:4]))

	var second bytes.Buffer
	assert.Equal(t, 0, RunCLI([]string{"--file", path, "--query", "PRINT M"}, &second, &errOut))
	assert.Equal(t, "5\n", second.String())
}

func TestCLIReadsBinaryFileWrittenByCpp(t *testing.T) {
	path := filepath.Join(t.TempDir(), "cpp.bin")
	require.NoError(t, os.WriteFile(path, cppDump(t), 0o600))
	var out, errOut bytes.Buffer
	assert.Equal(t, 0, RunCLI([]string{"--file", path, "--query", "HGET apple"}, &out, &errOut))
	assert.Equal(t, "3\n", out.String())
}

func TestCLIReportsCorruptedBinaryFile(t *testing.T) {
	path := filepath.Join(t.TempDir(), "broken.bin")
	require.NoError(t, os.WriteFile(path, []byte("garbage data"), 0o600))
	var out, errOut bytes.Buffer
	assert.Equal(t, 2, RunCLI([]string{"--file", path, "--query", "MLEN"}, &out, &errOut))
	assert.Contains(t, errOut.String(), "ERROR")
}
