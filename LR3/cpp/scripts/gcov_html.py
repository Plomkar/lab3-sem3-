import argparse
import gzip
import html
import json
import os
import subprocess
import sys
from collections import defaultdict


def collect(build_dir, root):
    coverage = defaultdict(lambda: defaultdict(int))
    for dirpath, _, names in os.walk(build_dir):
        for name in names:
            if not name.endswith(".gcda"):
                continue
            result = subprocess.run(
                ["gcov", "--json-format", "--stdout", os.path.join(dirpath, name)],
                cwd=dirpath,
                capture_output=True,
                check=False,
            )
            raw = result.stdout
            if not raw:
                continue
            for chunk in raw.splitlines():
                if not chunk.strip().startswith(b"{"):
                    continue
                data = json.loads(chunk)
                for entry in data.get("files", []):
                    path = entry["file"]
                    if not os.path.isabs(path):
                        path = os.path.join(data.get("current_working_directory", dirpath), path)
                    path = os.path.realpath(path)
                    if not path.startswith(root + os.sep):
                        continue
                    rel = os.path.relpath(path, root)
                    for line in entry.get("lines", []):
                        coverage[rel][line["line_number"]] += line["count"]
    return coverage


def keep(rel, includes, excludes):
    if not any(rel.startswith(prefix) for prefix in includes):
        return False
    return not any(rel.startswith(prefix) for prefix in excludes)


def percent(covered, total):
    return 100.0 * covered / total if total else 100.0


def color_class(value):
    if value >= 85:
        return "high"
    if value >= 60:
        return "mid"
    return "low"


STYLE = """
body{font-family:Segoe UI,Arial,sans-serif;margin:24px;color:#1f2933}
h1{font-size:22px;margin-bottom:4px}
table{border-collapse:collapse;width:100%;margin-top:12px}
th,td{border:1px solid #cfd6dd;padding:6px 10px;text-align:left;font-size:14px}
th{background:#eef2f6}
.bar{display:inline-block;height:10px;background:#d9534f;width:140px;vertical-align:middle;border-radius:2px}
.bar span{display:block;height:10px;background:#3fa34d;border-radius:2px}
.high{color:#1b7a2b;font-weight:600}.mid{color:#b57600;font-weight:600}.low{color:#b02a2a;font-weight:600}
pre{margin:0;font-family:Cascadia Mono,Consolas,monospace;font-size:13px}
.src td{border:none;padding:0 8px;white-space:pre}
.src .n{color:#7b8794;text-align:right;width:48px}
.src .c{text-align:right;width:70px;color:#52606d}
.hit{background:#e3f6e6}.miss{background:#fde2e2}
.summary{display:flex;gap:24px;margin:12px 0}
.card{border:1px solid #cfd6dd;border-radius:6px;padding:10px 16px}
.card b{display:block;font-size:24px}
"""


def render_file(rel, root, lines, out_dir):
    with open(os.path.join(root, rel), encoding="utf-8") as handle:
        source = handle.read().splitlines()
    covered = sum(1 for v in lines.values() if v > 0)
    total = len(lines)
    rows = []
    for index, text in enumerate(source, start=1):
        if index in lines:
            count = lines[index]
            cls = "hit" if count > 0 else "miss"
            shown = str(count)
        else:
            cls = ""
            shown = ""
        rows.append(
            '<tr class="%s"><td class="n">%d</td><td class="c">%s</td><td><pre>%s</pre></td></tr>'
            % (cls, index, shown, html.escape(text))
        )
    value = percent(covered, total)
    page = (
        "<!doctype html><html><head><meta charset='utf-8'><title>%s</title><style>%s</style></head><body>"
        "<h1>%s</h1><p><a href='index.html'>&larr; к списку файлов</a></p>"
        "<p>Покрытие строк: <span class='%s'>%.1f%%</span> (%d из %d)</p>"
        "<table class='src'>%s</table></body></html>"
        % (html.escape(rel), STYLE, html.escape(rel), color_class(value), value, covered, total, "".join(rows))
    )
    name = rel.replace(os.sep, "_") + ".html"
    with open(os.path.join(out_dir, name), "w", encoding="utf-8") as handle:
        handle.write(page)
    return name, covered, total


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--build", required=True)
    parser.add_argument("--root", default=".")
    parser.add_argument("--out", required=True)
    parser.add_argument("--include", action="append", default=["include/", "src/"])
    parser.add_argument("--exclude", action="append", default=["src/main.cpp"])
    parser.add_argument("--fail-under", type=float, default=0.0)
    args = parser.parse_args()

    root = os.path.realpath(args.root)
    os.makedirs(args.out, exist_ok=True)
    data = collect(os.path.realpath(args.build), root)
    files = sorted(rel for rel in data if keep(rel, args.include, args.exclude))

    rows = []
    covered_all = 0
    total_all = 0
    for rel in files:
        name, covered, total = render_file(rel, root, data[rel], args.out)
        covered_all += covered
        total_all += total
        value = percent(covered, total)
        rows.append(
            "<tr><td><a href='%s'>%s</a></td><td>%d</td><td>%d</td>"
            "<td><span class='bar'><span style='width:%.0f%%'></span></span></td><td class='%s'>%.1f%%</td></tr>"
            % (name, html.escape(rel), total, covered, value, color_class(value), value)
        )
    overall = percent(covered_all, total_all)
    index = (
        "<!doctype html><html><head><meta charset='utf-8'><title>Отчёт о покрытии</title><style>%s</style></head><body>"
        "<h1>Отчёт о покрытии кода тестами</h1>"
        "<div class='summary'><div class='card'>Покрытие строк<b class='%s'>%.1f%%</b></div>"
        "<div class='card'>Строк кода<b>%d</b></div><div class='card'>Покрыто<b>%d</b></div></div>"
        "<table><tr><th>Файл</th><th>Строк</th><th>Покрыто</th><th></th><th>Процент</th></tr>%s</table></body></html>"
        % (STYLE, color_class(overall), overall, total_all, covered_all, "".join(rows))
    )
    with open(os.path.join(args.out, "index.html"), "w", encoding="utf-8") as handle:
        handle.write(index)

    print("%-40s %6s %8s %8s" % ("file", "lines", "covered", "percent"))
    for rel in files:
        lines = data[rel]
        covered = sum(1 for v in lines.values() if v > 0)
        print("%-40s %6d %8d %7.1f%%" % (rel, len(lines), covered, percent(covered, len(lines))))
    print("%-40s %6d %8d %7.1f%%" % ("TOTAL", total_all, covered_all, overall))
    if overall < args.fail_under:
        print("coverage %.1f%% is below %.1f%%" % (overall, args.fail_under), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
