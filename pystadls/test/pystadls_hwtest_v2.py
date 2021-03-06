#!/usr/bin/env python

import unittest
import pyhalco_common as Co
import pyhalco_hicann_dls_v2 as C
import pyhaldls_v2 as Ct
import pystadls_v2 as IO

import pylogging as logger

OFFSET = 1000  # ~10 us
ISI = 2000  # ~20 us
NUM_SPIKES = 50


class TestPyhaldlsIOV2Hardware(unittest.TestCase):
    @unittest.skip("Issue #3243")
    def test_playback(self):
        capmem_config = Ct.CapMem()
        for cell in C.iter_all(C.CapMemCellOnDLS):
            capmem_config.set(cell,
                              cell.toEnum().value())

        cell = C.CapMemCellOnDLS(Co.Enum(2))
        capmemvalue = 334
        capmem_config.set(
            C.CapMemCellOnDLS(Co.Enum(0)), 123)
        capmem_config.set(
            C.CapMemCellOnDLS(Co.Enum(1)), 321)
        capmem_config.set(cell,
                          capmemvalue)

        builder = Ct.PlaybackProgramBuilder()
        builder.write(C.CapMemOnDLS(),
                      capmem_config)
        builder.wait_until(100)
        capmem_ticket = builder.read(C.CapMemOnDLS())
        capmemcell_ticket = builder.read(cell)
        builder.halt()

        program = builder.done()

        # No data available yet
        with self.assertRaises(RuntimeError):
            capmem_ticket.get()
        with self.assertRaises(RuntimeError):
            capmemcell_ticket.get()

        ctrl = IO.ExperimentControl()
        ctrl.run_experiment(Ct.Board(),
                            Ct.Chip(), program)

        capmem_copy = capmem_ticket.get()
        capmemcell_copy = capmemcell_ticket.get()

        self.assertEqual(capmem_config,
                         capmem_copy)
        self.assertEqual(capmemvalue,
                         capmemcell_copy.value)


class TestHelloWorldHardware(unittest.TestCase):
    def setUp(self):
        self.weight = 63
        self.address = 42
        self.neuron = C.NeuronOnDLS(0)
        self.synapse_driver = C.SynapseDriverOnDLS(0)
        self.chip = Ct.Chip()
        self.board = Ct.Board()

    def configure_dac(self):
        self.board.set_parameter(Ct.Board.Parameter.cadc_ramp_bias, 2730)
        self.board.set_parameter(Ct.Board.Parameter.cadc_ramp_01, 1365)
        self.board.set_parameter(Ct.Board.Parameter.cadc_ramp_slope, 1350)
        self.board.set_parameter(Ct.Board.Parameter.cadc_v_bias, 2389)
        self.board.set_parameter(Ct.Board.Parameter.syn_v_dd_res_meas, 4095)
        self.board.set_parameter(Ct.Board.Parameter.syn_v_store, 1200)
        self.board.set_parameter(Ct.Board.Parameter.syn_v_ramp, 1200)
        self.board.set_parameter(Ct.Board.Parameter.syn_v_bias, 1500)
        self.board.set_parameter(Ct.Board.Parameter.capmem_i_offset, 0)
        self.board.set_parameter(Ct.Board.Parameter.general_purpose_0, 0)
        self.board.set_parameter(Ct.Board.Parameter.general_purpose_1, 0)
        self.board.set_parameter(Ct.Board.Parameter.syn_v_reset, 3700)
        self.board.set_parameter(Ct.Board.Parameter.syn_corout_bias, 573)
        self.board.set_parameter(Ct.Board.Parameter.capmem_i_buf_bias, 3000)
        self.board.set_parameter(Ct.Board.Parameter.capmem_i_ref, 3906)

    def configure_capmem(self):
        capmem_config = self.chip.capmem_config
        capmem_config.enable_capmem = True
        self.chip.capmem_config = capmem_config

        # Set the cap mem
        capmem_config = self.chip.capmem
        capmem_config.set(self.neuron,
                          C.NeuronParameter.v_leak, 400)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.v_treshold, 600)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.v_exc_syn_input_reference, 670)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.v_inh_syn_input_reference, 690)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.i_bias_spike_comparator, 650)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.i_spike_comparator_delay, 130)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.i_bias_leak_main, 200)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.i_bias_leak_sd, 500)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.i_bias_readout_buffer, 1022)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.i_refractory_time, 300)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.i_bias_exc_syn_input_main, 1022)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.i_bias_exc_syn_input_sd, 1022)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.i_bias_exc_syn_input_resistor, 200)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.i_bias_exc_syn_input_offset, 650)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.i_bias_inh_syn_input_resistor, 200)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.i_bias_inh_syn_input_main, 1022)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.i_bias_inh_syn_input_sd, 1022)
        capmem_config.set(self.neuron,
                          C.NeuronParameter.i_bias_inh_syn_input_offset, 400)
        capmem_config.set(C.CommonNeuronParameter.e_reset, 300)
        self.chip.capmem = capmem_config

    def configure(self):
        self.configure_dac()
        self.configure_capmem()

        # Set connectivity
        synapse = C.SynapseOnDLS(self.neuron.toSynapseColumnOnDLS(),
                                 self.synapse_driver.toSynapseRowOnDLS())
        synapse_config = self.chip.get_synapse(synapse)
        synapse_config.weight = self.weight
        synapse_config.address = self.address
        self.chip.set_synapse(synapse, synapse_config)
        common_synram_config = self.chip.common_synram_config
        common_synram_config.pc_conf = 1
        common_synram_config.w_conf = 1
        common_synram_config.wait_ctr_clear = 1
        self.chip.common_synram_config = common_synram_config

        # Set current switches at the synaptic input
        current_switch = self.neuron.toColumnCurrentSwitchOnDLS()
        switch_config = self.chip.get_column_current_switch(current_switch)
        switch_config.inh_config = \
            Ct.ColumnCurrentBlock.ColumnCurrentSwitch.Config.disabled
        switch_config.exc_config = \
            Ct.ColumnCurrentBlock.ColumnCurrentSwitch.Config.internal
        self.chip.set_column_current_switch(current_switch, switch_config)

        # Set synapse driver
        syndrv_config = self.chip.synapse_drivers
        syndrv_config.set_mode(self.synapse_driver,
                               Ct.SynapseDriverBlock.Mode.excitatory)
        syndrv_config.pulse_length = 8
        self.chip.synapse_drivers = syndrv_config

        common_neuron_config = self.chip.common_neuron_config
        common_neuron_config.enable_digital_out = True
        self.chip.common_neuron_config = common_neuron_config

        # Set neuron's debug output and disable spike output
        neuron_config = self.chip.get_neuron_digital_config(self.neuron)
        neuron_config.fire_out_mode = \
            Ct.NeuronDigitalConfig.FireOutMode.disabled
        neuron_config.mux_readout_mode = \
            Ct.NeuronDigitalConfig.MuxReadoutMode.v_mem
        self.chip.set_neuron_digital_config(self.neuron, neuron_config)
        self.chip.enable_buffered_readout(self.neuron)

    def run_program(self, program):
        ctrl = IO.ExperimentControl()
        ctrl.run_experiment(self.board, self.chip, program)

    def test_silence(self):
        builder = Ct.PlaybackProgramBuilder()
        builder.wait_for(10000)
        builder.halt()
        program = builder.done()
        self.configure()
        self.run_program(program)
        spikes = program.get_spikes()
        self.assertEqual(len(spikes), 0)

    def configure_spiking(self):
        self.configure()

        neuron_config = self.chip.get_neuron_digital_config(self.neuron)
        neuron_config.enable_synapse_input_excitatory = True
        neuron_config.enable_synapse_input_inhibitory = True
        self.chip.set_neuron_digital_config(self.neuron, neuron_config)

        # Create a playback program (all times are in FPGA cycles / 96MHz)
        builder = Ct.PlaybackProgramBuilder()
        builder.set_time(0)
        for idx in range(NUM_SPIKES):
            builder.wait_until(OFFSET + idx * ISI)  # Absolute wait
            builder.fire(self.synapse_driver, self.address)
        builder.wait_for(OFFSET)  # Relative wait
        builder.halt()
        program = builder.done()

        return program

    def check_spikes(self, spikes):
        self.assertEqual(len(spikes), NUM_SPIKES)

        last_time = 0
        for spike in spikes:
            time = spike.time
            actual_neuron = spike.neuron
            self.assertEqual(actual_neuron, self.neuron)
            if last_time > 0:
                self.assertAlmostEqual(time - last_time, ISI, delta=10)
            last_time = time

    def test_bypass(self):
        program = self.configure_spiking()

        neuron_config = self.chip.get_neuron_digital_config(self.neuron)
        neuron_config.fire_out_mode = \
            Ct.NeuronDigitalConfig.FireOutMode.bypass_exc
        self.chip.set_neuron_digital_config(self.neuron, neuron_config)

        self.run_program(program)
        self.check_spikes(program.get_spikes())


if __name__ == "__main__":
    logger.reset()
    logger.default_config(level=logger.LogLevel.DEBUG)
    unittest.main()
