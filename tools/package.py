#Package only explicit submission files, never Git data or build artifacts.
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile


ROOT = Path(__file__).resolve().parents[1]


def main():
    #Create a ZIP with a single root directory and all 21 original inputs.
    output = ROOT / "dist/entrega2_wifi.zip"
    files = sorted(ROOT.glob("src/*.c")) + sorted(ROOT.glob("include/*.h"))
    files += [ROOT / name for name in (
        "Makefile", "README.md", ".gitignore", ".gitattributes",
        "tools/package.py",
    )]
    files += [ROOT / f"cells/info_cell_{cell}.txt" for cell in range(1, 22)]
    for path in files:
        if not path.is_file():
            raise FileNotFoundError(path)
    output.parent.mkdir(exist_ok=True)
    with ZipFile(output, "w", ZIP_DEFLATED) as archive:
        for path in files:
            name = "entrega2_wifi/" + path.relative_to(ROOT).as_posix()
            archive.write(path, name)
    print(output)


if __name__ == "__main__":
    main()
