"""Run parser regressions and keyboard-driven application scenarios."""

from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


def run_program(text, cwd=ROOT):
    """Supply terminal input and fail on crashes or non-terminating loops."""
    return subprocess.run(
        [str(ROOT / "main")], input=text, text=True, encoding="utf-8",
        capture_output=True, timeout=10, check=True, cwd=cwd,
    ).stdout


class CollectorTests(unittest.TestCase):
    """Use real input files and isolated malformed fixtures."""

    def test_corpus_atomicity_and_capacity(self):
        base = (ROOT / "cells/info_cell_21.txt").read_text()
        base = base.replace("Cell 21", "Cell 1").rstrip() + "\n"
        fixtures = {
            "empty": "", "blank": "\n  \n\t\n",
            "truncated": "Cell 1\nAddress: 00:27:0D:56:0B:90\n",
            "bad_mac": base.replace("00:27:0D:56:0B:90", "invalid"),
            "bad_mode": base.replace("Mode:Master", "Mode:Invalid"),
            "bad_encryption": base.replace("key:off", "key:yes"),
            "bad_quality": base.replace("31/70", "71/70"),
            "zero_quality": base.replace("31/70", "0/0"),
            "bad_channel": base.replace("Channel:11", "Channel:11x"),
            "bad_essid": base.replace('ESSID:"WiFi-UC3M"', 'ESSID:bad'),
            "wrong_cell": base.replace("Cell 1", "Cell 2"),
            "duplicate": base + "Channel:11\n",
            "long_line": base + "x" * 80 + "\n",
            "partial": base + "Cell 1\nESSID:bad\n",
            "too_many": base * 201,
            "unknown_field": base + "Unexpected:value\n",
            "bad_number": base.replace("Channel:11", "Channel:" + "9" * 60),
            "no_newline": base.rstrip(),
            "valid_modes": "".join(
                base.replace("Mode:Master", "Mode:" + mode)
                for mode in ("Auto", "Ad-Hoc", "Managed", "Master",
                             "Repeater", "Secondary", "Monitor", "Unknown")
            ),
        }
        with tempfile.TemporaryDirectory(prefix="wifi-") as directory:
            for name, content in fixtures.items():
                (Path(directory) / (name + ".txt")).write_text(content)
            (Path(directory) / "crlf.txt").write_bytes(
                base.replace("\n", "\r\n").encode()
            )
            result = subprocess.run(
                [str(ROOT / "build/test_collector"), directory],
                capture_output=True, text=True, timeout=10,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("Collector checks passed", result.stdout)


class TerminalTests(unittest.TestCase):
    """Validate observable behavior instead of implementation details."""

    def test_empty_display_and_quit_cancel(self):
        output = run_program("10\n1\nN\n1\ns\n")
        self.assertIn("No hay celdas almacenadas", output)
        self.assertEqual(output.count("Opcion elegida:"), 3)
        self.assertEqual(output.count("Programa terminado."), 1)

    def test_invalid_input_and_confirmations(self):
        output = run_program(
            "\nhello\n2x\n0\n11\n3\n2\n\nabc\n0\n22\n1\n"
            "\ny\nS\nn\nN\n10\n1\nN \ns\n"
        )
        self.assertIn("Opcion no disponible", output)
        self.assertIn("Introduzca un entero entre 1 y 21", output)
        self.assertIn("Responda exactamente s o N", output)
        self.assertEqual(output.count("Cell 1\n"), 3)

    def test_long_lines_are_drained(self):
        output = run_program("9" * 300 + "\n" + "1" + " " * 79 + "\n1\ns\n")
        self.assertEqual(output.count("Entrada demasiado larga"), 2)
        self.assertEqual(output.count("Esta seguro"), 1)

    def test_maximum_length_line(self):
        output = run_program("1" + " " * 78 + "\ns\n")
        self.assertNotIn("Entrada demasiado larga", output)
        self.assertEqual(output.count("Esta seguro"), 1)

    def test_display_one_and_repeat_after_missing_cell(self):
        output = run_program("2\n1\nN\n9\n21\ns\n1\nN\n1\ns\n")
        self.assertIn("celda 21 no ha sido recolectada", output)
        self.assertEqual(output.count("Desea imprimir otra celda"), 2)
        self.assertEqual(output.count("Cell 1\n"), 3)
        self.assertNotIn("Frequency:", output)
        self.assertNotIn("Signal level=", output)

    def test_all_cells_and_recollection(self):
        requests = "2\n" + "s\n".join(str(i) + "\n" for i in range(1, 22))
        output = run_program(requests + "N\n10\n2\n21\nN\n10\n1\ns\n")
        self.assertEqual(output.count("Address: "), 47 + 48)
        self.assertIn('ESSID:"Miguel 3"', output)
        self.assertIn("Channel:140", output)
        self.assertIn("Mode:Ad-Hoc", output)

    def test_missing_file_returns_to_menu(self):
        with tempfile.TemporaryDirectory(prefix="wifi-") as directory:
            output = run_program("2\n1\nN\n10\n1\ns\n", cwd=directory)
        self.assertIn("Error al abrir", output)
        self.assertIn("No hay celdas almacenadas", output)

    def test_eof_at_every_prompt(self):
        for text in ("", "1\n", "2\n", "2\n1\n", "9\n", "9\n1\n"):
            with self.subTest(text=text):
                self.assertIn("Programa terminado.", run_program(text))


if __name__ == "__main__":
    unittest.main(verbosity=2)
