package ds

import (
	"bytes"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestDatabaseEmptyAndUnknownQueries(t *testing.T) {
	database := NewDatabase()
	assert.Equal(t, "ERROR: empty query", database.Execute(""))
	assert.Equal(t, "ERROR: empty query", database.Execute("   "))
	assert.Equal(t, "Unknown command: BOOM", database.Execute("BOOM 1"))
}

func TestDatabaseArrayCommands(t *testing.T) {
	database := NewDatabase()
	assert.Equal(t, "5", database.Execute("MPUSH 5"))
	assert.Equal(t, "7", database.Execute("MPUSH 7"))
	assert.Equal(t, "6", database.Execute("MPUSH 1 6"))
	assert.Equal(t, "5 6 7", database.Execute("PRINT M"))
	assert.Equal(t, "6", database.Execute("MGET 1"))
	assert.Equal(t, "OK", database.Execute("MSET 1 60"))
	assert.Equal(t, "OK", database.Execute("MDEL 0"))
	assert.Equal(t, "2", database.Execute("MLEN"))
	assert.Equal(t, "60 7", database.Execute("PRINT M"))
	for _, query := range []string{"MGET 9", "MPUSH -1 3", "MGET -1", "MGET", "MGET abc", "MPUSH", "MPUSH 1 x", "MSET 1", "MSET x 1", "MSET 9 1", "MDEL", "MDEL 9"} {
		assert.True(t, strings.HasPrefix(database.Execute(query), "ERROR:"), query)
	}
}

func TestDatabaseForwardListCommands(t *testing.T) {
	database := NewDatabase()
	assert.Equal(t, "OK", database.Execute("FPUSH 2"))
	assert.Equal(t, "OK", database.Execute("FPUSH 1"))
	assert.Equal(t, "OK", database.Execute("FPUSH_END 4"))
	assert.Equal(t, "OK", database.Execute("FPUSH 2 3"))
	assert.Equal(t, "OK", database.Execute("FPUSH_AFTER 3 40"))
	assert.Equal(t, "1 2 3 4 40", database.Execute("PRINT F"))
	assert.Equal(t, "3", database.Execute("FGET 2"))
	assert.Equal(t, "3", database.Execute("FSEARCH 4"))
	assert.Equal(t, "-1", database.Execute("FSEARCH 99"))
	assert.Equal(t, "OK", database.Execute("FDEL 0"))
	assert.Equal(t, "OK", database.Execute("FDEL_VALUE 40"))
	assert.Equal(t, "NOT_FOUND", database.Execute("FDEL_VALUE 40"))
	assert.Equal(t, "2 3 4", database.Execute("PRINT F"))
	for _, query := range []string{"FPUSH -1 5", "FPUSH", "FPUSH 1 x", "FPUSH_END", "FPUSH_AFTER 1", "FPUSH_AFTER 99 1", "FGET 10", "FGET", "FDEL 10", "FDEL", "FDEL_VALUE", "FSEARCH"} {
		assert.True(t, strings.HasPrefix(database.Execute(query), "ERROR:"), query)
	}
}

func TestDatabaseLinkedListCommands(t *testing.T) {
	database := NewDatabase()
	assert.Equal(t, "OK", database.Execute("LPUSH 2"))
	assert.Equal(t, "OK", database.Execute("LPUSH 1"))
	assert.Equal(t, "OK", database.Execute("LPUSH_END 4"))
	assert.Equal(t, "OK", database.Execute("LPUSH 2 3"))
	assert.Equal(t, "OK", database.Execute("LPUSH_AFTER 3 40"))
	assert.Equal(t, "1 2 3 4 40", database.Execute("PRINT L"))
	assert.Equal(t, "40 4 3 2 1", database.Execute("PRINT LR"))
	assert.Equal(t, "3", database.Execute("LGET 2"))
	assert.Equal(t, "3", database.Execute("LSEARCH 4"))
	assert.Equal(t, "-1", database.Execute("LSEARCH 99"))
	assert.Equal(t, "OK", database.Execute("LDEL 0"))
	assert.Equal(t, "OK", database.Execute("LDEL_VALUE 40"))
	assert.Equal(t, "NOT_FOUND", database.Execute("LDEL_VALUE 40"))
	assert.Equal(t, "2 3 4", database.Execute("PRINT L"))
	for _, query := range []string{"LPUSH -1 5", "LPUSH", "LPUSH 1 x", "LPUSH_END", "LPUSH_AFTER 1", "LPUSH_AFTER 99 1", "LGET 10", "LGET", "LDEL 10", "LDEL", "LDEL_VALUE", "LSEARCH"} {
		assert.True(t, strings.HasPrefix(database.Execute(query), "ERROR:"), query)
	}
}

func TestDatabaseStackAndQueueCommands(t *testing.T) {
	database := NewDatabase()
	assert.Equal(t, "1", database.Execute("SPUSH 1"))
	assert.Equal(t, "2", database.Execute("SPUSH 2"))
	assert.Equal(t, "2", database.Execute("SPEEK"))
	assert.Equal(t, "2 1", database.Execute("PRINT S"))
	assert.Equal(t, "2", database.Execute("SPOP"))
	assert.Equal(t, "1", database.Execute("SPOP"))
	assert.True(t, strings.HasPrefix(database.Execute("SPOP"), "ERROR:"))
	assert.True(t, strings.HasPrefix(database.Execute("SPEEK"), "ERROR:"))
	assert.True(t, strings.HasPrefix(database.Execute("SPUSH"), "ERROR:"))

	assert.Equal(t, "1", database.Execute("QPUSH 1"))
	assert.Equal(t, "2", database.Execute("QPUSH 2"))
	assert.Equal(t, "1", database.Execute("QPEEK"))
	assert.Equal(t, "1 2", database.Execute("PRINT Q"))
	assert.Equal(t, "1", database.Execute("QPOP"))
	assert.Equal(t, "2", database.Execute("QPOP"))
	assert.True(t, strings.HasPrefix(database.Execute("QPOP"), "ERROR:"))
	assert.True(t, strings.HasPrefix(database.Execute("QPEEK"), "ERROR:"))
	assert.True(t, strings.HasPrefix(database.Execute("QPUSH"), "ERROR:"))
}

func TestDatabaseTreeCommands(t *testing.T) {
	database := NewDatabase()
	for i := 1; i <= 7; i++ {
		assert.Equal(t, strconv.Itoa(i), database.Execute("TINSERT "+strconv.Itoa(i)))
	}
	assert.Equal(t, "4", database.Execute("TGET 3"))
	assert.Equal(t, "4", database.Execute("TSEARCH 5"))
	assert.Equal(t, "-1", database.Execute("TSEARCH 99"))
	assert.Equal(t, "TRUE", database.Execute("TFULL"))
	assert.Equal(t, "3", database.Execute("THEIGHT"))
	assert.Equal(t, "1 2 4 5 3 6 7", database.Execute("TORDER pre"))
	assert.Equal(t, "4 2 5 1 6 3 7", database.Execute("TORDER in"))
	assert.Equal(t, "4 5 2 6 7 3 1", database.Execute("TORDER post"))
	assert.Equal(t, "1 2 3 4 5 6 7", database.Execute("TORDER level"))
	assert.Equal(t, "OK", database.Execute("TDEL 0"))
	assert.Equal(t, "FALSE", database.Execute("TFULL"))
	assert.Equal(t, "7 2 3 4 5 6", database.Execute("PRINT T"))
	for _, query := range []string{"TORDER zigzag", "TORDER", "TGET 20", "TGET", "TDEL 20", "TDEL", "TINSERT", "TSEARCH"} {
		assert.True(t, strings.HasPrefix(database.Execute(query), "ERROR:"), query)
	}
}

func TestDatabaseHashTableCommands(t *testing.T) {
	database := NewDatabase()
	assert.Equal(t, "OK", database.Execute("HPUT apple 3"))
	assert.Equal(t, "OK", database.Execute("HPUT pear 5"))
	assert.Equal(t, "3", database.Execute("HGET apple"))
	assert.Equal(t, "NOT_FOUND", database.Execute("HGET plum"))
	assert.Equal(t, "TRUE", database.Execute("HHAS pear"))
	assert.Equal(t, "FALSE", database.Execute("HHAS plum"))
	assert.Equal(t, "2", database.Execute("HSIZE"))
	assert.Equal(t, "apple=3 pear=5", database.Execute("PRINT H"))
	assert.Equal(t, "TRUE", database.Execute("HDEL apple"))
	assert.Equal(t, "FALSE", database.Execute("HDEL apple"))
	for _, query := range []string{"HPUT", "HPUT key", "HPUT key x", "HGET", "HDEL", "HHAS"} {
		assert.True(t, strings.HasPrefix(database.Execute(query), "ERROR:"), query)
	}
}

func TestDatabasePrintValidatesContainerName(t *testing.T) {
	database := NewDatabase()
	assert.True(t, strings.HasPrefix(database.Execute("PRINT X"), "ERROR:"))
	assert.True(t, strings.HasPrefix(database.Execute("PRINT"), "ERROR:"))
}

func TestDatabaseStatsUsesInterface(t *testing.T) {
	database := NewDatabase()
	database.Execute("MPUSH 1")
	database.Execute("SPUSH 2")
	database.Execute("SPUSH 3")
	stats := database.Execute("STATS")
	assert.Contains(t, stats, "Array: 1")
	assert.Contains(t, stats, "Stack: 2")
	assert.Contains(t, stats, "HashTable: 0")
	assert.Len(t, database.Containers(), 7)
}

func TestDatabaseAccessors(t *testing.T) {
	database := NewDatabase()
	database.Execute("MPUSH 4")
	database.Execute("FPUSH 5")
	database.Execute("LPUSH 6")
	database.Execute("SPUSH 7")
	database.Execute("QPUSH 8")
	database.Execute("TINSERT 9")
	database.Execute("HPUT k 10")
	assert.Equal(t, 1, database.Array().Len())
	assert.Equal(t, 1, database.ForwardList().Len())
	assert.Equal(t, 1, database.LinkedList().Len())
	assert.Equal(t, 1, database.Stack().Len())
	assert.Equal(t, 1, database.Queue().Len())
	assert.Equal(t, 1, database.Tree().Len())
	assert.Equal(t, 1, database.HashTable().Len())
}

func fillDatabase(database *Database) {
	for _, query := range []string{
		"MPUSH 1", "MPUSH 2", "FPUSH 3", "LPUSH_END 4", "SPUSH 5", "QPUSH 6", "TINSERT 7", "HPUT key 8",
	} {
		database.Execute(query)
	}
}

func TestDatabaseJSONRoundTrip(t *testing.T) {
	source := NewDatabase()
	fillDatabase(source)
	data, err := source.Marshal()
	require.NoError(t, err)

	loaded := NewDatabase()
	loaded.Execute("MPUSH 99")
	require.NoError(t, loaded.Unmarshal(data))
	assert.Equal(t, "1 2", loaded.Execute("PRINT M"))
	assert.Equal(t, "3", loaded.Execute("PRINT F"))
	assert.Equal(t, "4", loaded.Execute("PRINT L"))
	assert.Equal(t, "5", loaded.Execute("PRINT S"))
	assert.Equal(t, "6", loaded.Execute("PRINT Q"))
	assert.Equal(t, "7", loaded.Execute("PRINT T"))
	assert.Equal(t, "8", loaded.Execute("HGET key"))
}

func TestDatabaseUnmarshalRejectsBadData(t *testing.T) {
	database := NewDatabase()
	assert.ErrorIs(t, database.Unmarshal([]byte("not json")), ErrFormat)
	assert.ErrorIs(t, database.Unmarshal([]byte(`{"version":99}`)), ErrFormat)
	assert.ErrorIs(t, database.Unmarshal([]byte(`{"version":1}`)), ErrFormat)
	assert.ErrorIs(t, database.Unmarshal([]byte(`{"version":1,"array":"oops"}`)), ErrFormat)
}

func TestDatabaseFileRoundTrip(t *testing.T) {
	path := filepath.Join(t.TempDir(), "db.json")
	source := NewDatabase()
	fillDatabase(source)
	require.NoError(t, source.SaveToFile(path))

	loaded := NewDatabase()
	require.NoError(t, loaded.LoadFromFile(path))
	assert.Equal(t, "1", loaded.Execute("MGET 0"))
	assert.Equal(t, "8", loaded.Execute("HGET key"))
}

func TestDatabaseFileErrors(t *testing.T) {
	database := NewDatabase()
	dir := t.TempDir()
	assert.Error(t, database.LoadFromFile(filepath.Join(dir, "missing.json")))
	assert.Error(t, database.SaveToFile(filepath.Join(dir, "no_such_dir", "db.json")))
}

func TestCLIPrintsUsageWithoutQuery(t *testing.T) {
	var out, errOut bytes.Buffer
	assert.Equal(t, 0, RunCLI(nil, &out, &errOut))
	assert.Contains(t, out.String(), "Usage")
	out.Reset()
	assert.Equal(t, 0, RunCLI([]string{"--query"}, &out, &errOut))
	assert.Contains(t, out.String(), "Usage")
}

func TestCLIRunsQueriesAgainstFile(t *testing.T) {
	path := filepath.Join(t.TempDir(), "cli.json")
	var out, errOut bytes.Buffer
	assert.Equal(t, 0, RunCLI([]string{"--file", path, "--query", "MPUSH 5"}, &out, &errOut))
	assert.Equal(t, 0, RunCLI([]string{"--file", path, "--query", "MPUSH 6"}, &out, &errOut))
	var second bytes.Buffer
	assert.Equal(t, 0, RunCLI([]string{"--file", path, "--query", "PRINT M"}, &second, &errOut))
	assert.Equal(t, "5 6\n", second.String())
}

func TestCLIWorksWithoutFile(t *testing.T) {
	var out, errOut bytes.Buffer
	assert.Equal(t, 0, RunCLI([]string{"--query", "MPUSH 1"}, &out, &errOut))
	assert.Equal(t, "1\n", out.String())
}

func TestCLIReportsQueryErrors(t *testing.T) {
	var out, errOut bytes.Buffer
	assert.Equal(t, 1, RunCLI([]string{"--query", "SPOP"}, &out, &errOut))
	assert.True(t, strings.HasPrefix(out.String(), "ERROR:"))
}

func TestCLIReportsCorruptedFile(t *testing.T) {
	path := filepath.Join(t.TempDir(), "broken.json")
	require.NoError(t, os.WriteFile(path, []byte("garbage"), 0o600))
	var out, errOut bytes.Buffer
	assert.Equal(t, 2, RunCLI([]string{"--file", path, "--query", "MLEN"}, &out, &errOut))
	assert.Contains(t, errOut.String(), "ERROR")
}

func TestCLIReportsSaveFailure(t *testing.T) {
	path := filepath.Join(t.TempDir(), "no_such_dir", "db.json")
	var out, errOut bytes.Buffer
	assert.Equal(t, 2, RunCLI([]string{"--file", path, "--query", "MPUSH 1"}, &out, &errOut))
	assert.Contains(t, errOut.String(), "ERROR")
}
