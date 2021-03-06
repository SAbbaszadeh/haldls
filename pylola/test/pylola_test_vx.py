#!/usr/bin/env python
import os
import unittest
import pylola_vx as lola
import pyhalco_hicann_dls_vx as halco


class TestPylolaVX(unittest.TestCase):
    def test_import(self):
        self.assertIsNotNone(lola)

    # pylint warnings disabled below because attributes for nested
    # classes are loaded dynamically upon instantiation and therefore
    # not visible for pylint (which is an error).
    # pylint: disable=E1101,E1136,E1137
    def test_synapse_row(self):
        row = lola.SynapseRow()

        row.weights[13] = 27
        self.assertEqual(row.weights[13], 27)
        np_weights = row.weights.to_numpy()
        self.assertEqual(np_weights[13], 27)
        self.assertEqual(np_weights.shape,
                         (halco.SynapseOnSynapseRow.size,))
        np_weights[16] = 64
        with self.assertRaisesRegex(RuntimeError,
                                    r"range overflow: (\d+) > max\((\d+)\)"):
            row.weights.from_numpy(np_weights)
        np_weights[16] = 4
        row.weights.from_numpy(np_weights)
        self.assertEqual(row.weights[16], 4)

        row.addresses[14] = 28
        self.assertEqual(row.addresses[14], 28)
        np_addresses = row.addresses.to_numpy()
        self.assertEqual(np_addresses[14], 28)
        self.assertEqual(np_addresses.shape,
                         (halco.SynapseOnSynapseRow.size,))
        np_addresses[16] = 64
        with self.assertRaisesRegex(RuntimeError,
                                    r"range overflow: (\d+) > max\((\d+)\)"):
            row.addresses.from_numpy(np_addresses)
        np_addresses[16] = 4
        row.addresses.from_numpy(np_addresses)
        self.assertEqual(row.addresses[16], 4)

        row.time_calibs[15] = 2
        self.assertEqual(row.time_calibs[15], 2)
        np_time_calibs = row.time_calibs.to_numpy()
        self.assertEqual(np_time_calibs[15], 2)
        self.assertEqual(np_time_calibs.shape,
                         (halco.SynapseOnSynapseRow.size,))
        np_time_calibs[16] = 4
        with self.assertRaisesRegex(RuntimeError,
                                    r"range overflow: (\d+) > max\((\d+)\)"):
            row.time_calibs.from_numpy(np_time_calibs)
        np_time_calibs[16] = 3
        row.time_calibs.from_numpy(np_time_calibs)
        self.assertEqual(row.time_calibs[16], 3)

        row.amp_calibs[16] = 3
        self.assertEqual(row.amp_calibs[16], 3)
        np_amp_calibs = row.amp_calibs.to_numpy()
        self.assertEqual(np_amp_calibs[16], 3)
        self.assertEqual(np_amp_calibs.shape,
                         (halco.SynapseOnSynapseRow.size,))
        np_amp_calibs[16] = 4
        with self.assertRaisesRegex(RuntimeError,
                                    r"range overflow: (\d+) > max\((\d+)\)"):
            row.amp_calibs.from_numpy(np_amp_calibs)
        np_amp_calibs[16] = 3
        row.amp_calibs.from_numpy(np_amp_calibs)
        self.assertEqual(row.amp_calibs[16], 3)

    def test_ppu_elf_file(self):
        this_dir = os.path.dirname(os.path.realpath(__file__))
        elf_file = lola.PPUElfFile(
            os.environ.get('TEST_PPU_PROGRAM',
                           os.path.join(this_dir,
                                        os.pardir,
                                        os.pardir,
                                        os.pardir,
                                        'build',
                                        'haldls',
                                        'lola_ppu_test_elf_file.bin')))
        # all numbers below might and will change on change in build-profile,
        # compiler or runtime
        program_size = 113
        self.assertEqual(elf_file.read_program().size(), program_size)

        symbols = elf_file.read_symbols()
        # get these numbers by powerpc-ppu-nm <program>
        symbol_a_position = halco.PPUMemoryBlockOnPPU(
            halco.PPUMemoryWordOnPPU(113), halco.PPUMemoryWordOnPPU(113))
        self.assertEqual(symbols["a"], lola.PPUProgram.Symbol(
            lola.PPUProgram.Symbol.Type.object, symbol_a_position))


if __name__ == "__main__":
    unittest.main()
