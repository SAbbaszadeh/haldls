
class iboard_ctrl(object):
    """iBoard controll class

    shamelessly copied from hicann-system/units/stage2_hal/source/iboardv2_ctrl.cpp
    """

    VDD11 = 'vdd11'
    VDD5 = 'vdd5'
    VDD25 = 'vdd25'
    VOH = 'voh'
    VOL = 'vol'
    VDDBUS = 'vddbus'
    VREFDAC = 'vrefdac'

    def __init__(self, iboards, cube):
        """
        iboards: 0x1 = FPGA-Sockel 0; 0x2 = FPGA-Sockel 3; 0x3 = both
        """
        if iboards < 0x0 or iboards > 0x3:
            raise ValueError('Invalid iboard')

        self.cube = cube
        self.iboards = iboards

        # set default values, taken from IBoardV2Ctrl::iboardConfigure(0)
        self.vdd11_r1 = 75000.0
        self.vdd11_r2 = 20000.0
        self.vdd5_r1 = 27000.0
        self.vdd5_r2 = 20000.0
        self.vddbus_r1 = 20000.0
        self.vddbus_r2 = 56000.0
        self.vdd25_r1 = 10000.0
        self.vdd25_r2 = 47000.0
        self.vol_r1 = 20000.0
        self.vol_r2 = 56000.0
        self.voh_r1 = 20000.0
        self.voh_r2 = 56000.0
        self.vref_r1 = 20000.0
        self.vref_r2 = 20000.0

        # instumentation amplifier external resistors:
        self.rg_vdd = 5360.0
        self.rg_vdddnc = 4000.0
        self.rg_vdda = 5360.0
        self.rg_vddadnc = 200.0
        self.rg_vdd33 = 200.0
        self.rg_vol = 200.0
        self.rg_voh = 200.0
        self.rg_vddbus = 1500.0
        self.rg_vdd25 = 200.0
        self.rg_vdd5 = 1500.0
        self.rg_vdd11 = 200.0

        # current measurement resistors
        self.rm_vdd = 0.01
        self.rm_vdddnc = 0.01
        self.rm_vdda = 0.01
        self.rm_vddadnc = 0.03
        self.rm_vdd33 = 0.03
        self.rm_vol = 0.01
        self.rm_voh = 0.01
        self.rm_vddbus = 0.01
        self.rm_vdd25 = 0.01
        self.rm_vdd5 = 0.01
        self.rm_vdd11 = 0.1

        # MUX configuration
        self.mux0addr = 0x9c
        self.mux1addr = 0x9e
        self.mux0cmd = 0x80
        self.mux1cmd = 0x80

        # DAC configuration
        self.vrefdac = 2.5
        self.dacres = 16
        self.dac0addr = 0x98
        self.dac1addr = 0x9a
        self.dacvol = 0x14
        self.dacvoh = 0x10
        self.dacvddbus = 0x12
        self.dacvdd25 = 0x10
        self.dacvdd5 = 0x12
        self.dacvdd11 = 0x14
        self.dacvref_dac = 0x16

        # ADC configuration
        self.vrefadc = 3.3
        self.vref_ina = 1.0
        self.adcres = 10
        self.adc0addr = 0x42
        self.adc1addr = 0x44
        self.readiter = 9
        self.adcivdda = 0xf0
        self.adcivdd_dncif = 0xb0
        self.adcivdda_dncif = 0xa0
        self.adcivdd33 = 0x80
        self.adcivol = 0xa0
        self.adcivoh = 0x90
        self.adcivdd = 0x90
        self.adcivddbus = 0x80
        self.adcivdd25 = 0xd0
        self.adcivdd5 = 0xc0
        self.adcivdd11 = 0xe0
        self.adcaout0 = 0xf0
        self.adcaout1 = 0xe0
        self.adcadc0 = 0xb0
        self.adcadc1 = 0xc0
        self.adcadc2 = 0xd0

        # Default voltages to be set with setAllVolt
        self.vdd5value = 5.0
        self.vdd25value = 2.5
        self.vdd11value = 10.5
        self.vohvalue = 0.9
        self.volvalue = 0.7
        self.vddbusvalue = 1.2

        # TODO needed?
        # log(Logger::DEBUG1) << "IBoardV2Ctrl instance created"

    def set_voltage(self, vname, value):
        """set one specific voltage, returns 0 on error"""

        if vname == self.VOL:
            dacaddr = self.dac1addr
            daccmd = self.dacvol
            dacvout = value * (self.vol_r1 + self.vol_r2) / self.vol_r2
        elif vname == self.VOH:
            dacaddr = self.dac1addr
            daccmd = self.dacvoh
            dacvout = value * (self.voh_r1 + self.voh_r2) / self.voh_r2
        elif vname == self.VDDBUS:
            dacaddr = self.dac1addr
            daccmd = self.dacvddbus
            dacvout = value * (self.vddbus_r1 + self.vddbus_r2) / self.vddbus_r2
        elif vname == self.VDD25:
            dacaddr = self.dac0addr
            daccmd = self.dacvdd25
            dacvout = value / (1 + self.vdd25_r1 / self.vdd25_r2)
        elif vname == self.VDD5:
            dacaddr = self.dac0addr
            daccmd = self.dacvdd5
            dacvout = value / (1 + self.vdd5_r1 / self.vdd5_r2)
        elif vname == self.VDD11:
            dacaddr = self.dac0addr
            daccmd = self.dacvdd11
            dacvout = value / (1 + self.vdd11_r1 / self.vdd11_r2)
        elif vname == self.VREFDAC:
            dacaddr = self.dac0addr
            daccmd = self.dacvref_dac
            dacvout = value * (self.vref_r1 + self.vddbus_r2) / self.vddbus_r2
        else:
            raise RuntimeError("Unkown voltage '{}' specified".format(vname))

        # sanity check:
        if dacvout > self.vrefdac:
            # log(Logger::ERROR) << "DAC input value larger than vrefdac!"
            return False

        dacval = int(dacvout / self.vrefdac * float((1<<self.dacres)-1))
        self.enable_i2c_master()
        self.i2c_write(dacaddr >> 1, daccmd, dacval, bytes=2)

    def set_both_mux(self):
        """set both MUX to standard values (both hicann0 analog outputs)"""
        self.switch_mux(7)
        # log(Logger::DEBUG0) << "Setting both MUX to default values"

    def switch_mux(self, muxid):
        """switch MUX to use a certain input line (0 to 7), default is 7"""
        if muxid < 0 or muxid > 7:
            raise ValueError("Invalid muxid")

        muxcmd = 0x1 << muxid

        self.enable_i2c_master()
        for addr in (self.mux0addr, self.mux1addr):
            self.i2c_write(addr >> 1, muxcmd, bytes=0)
            # log(Logger::INFO) << "Switching MUX " << muxid << " to input from channel " << channel

    def disable_mux(self):
        self.enable_i2c_master()
        for addr in (self.mux0addr, self.mux1addr):
            self.i2c_write(addr >> 1, 0x0, bytes=0)

    def set_defaults(self):
        """Mimic reticle_control"""
        self.set_both_mux()
        self.set_voltage(self.VDD11,  5.0)
        self.set_voltage(self.VDD5,   5.0)
        self.set_voltage(self.VDD25,  2.5)
        self.set_voltage(self.VDD11,  11.0)
        self.set_voltage(self.VOH,    0.9)
        self.set_voltage(self.VOL,    0.7)
        self.set_voltage(self.VDDBUS, 1.2)

    def power_down(self):
        """Mimic reticle_control"""
        self.disable_mux()
        self.set_voltage(self.VDDBUS, 0.0)
        self.set_voltage(self.VOL,    0.0)
        self.set_voltage(self.VOH,    0.0)
        self.set_voltage(self.VDD11,  0.0)
        self.set_voltage(self.VDD25,  0.0)
        self.set_voltage(self.VDD5,   0.0)

    def enable_i2c_master(self):
        """enable I2C master"""
        addr = 0x00
        if self.iboards & 0x1:
            addr |= 0x01
        if self.iboards & 0x2:
            addr |= 0x08
        self.i2c_write(saddr=0x70, addr=addr)

    def i2c_write(self, saddr, addr, data=0, bytes=0):
        # print "i2c_write({:#x}, {:#x}, {:#x}, {})".format(saddr, addr, data, bytes)
        if bytes == 0:
            # print "sendByteCommon(saddr={:#x}, data={:#x})".format(saddr, addr)
            self.cube.sendByteCommon(saddr=saddr, data=addr)
        elif bytes == 1:
            # print "writeByteCommon(saddr={:#x}, addr={:#x}, data={:#x})".format(saddr, addr, data)
            self.cube.writeByteCommon(saddr=saddr, addr=addr, data=data)
        elif bytes == 2:
            # print "writeWordCommon(saddr={:#x}, addr={:#x}, data={:#x})".format(saddr, addr, data)
            self.cube.writeWordCommon(saddr=saddr, addr=addr, data=data)
        else:
            raise RuntimeError("Invalid number of bytes")


if __name__ == '__main__':
    import pyhid_cube
    import pyhid
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument(
        '--iboard', choices=(1, 2, 3), type=int, required=True,
        help="Select iboard to configure, iboard 1 is connected to socket 1, "
             "iboard 2 to socket 4")
    subparsers = parser.add_subparsers(dest='command')
    command_set_voltage = subparsers.add_parser(
        'set_voltage', help="Set a iboard voltage to the requested value")
    command_set_voltage.add_argument('voltage', help="Voltage to set, e.g. voh")
    command_set_voltage.add_argument('value', type=float, help="Value in volts")
    command_set_voltage = subparsers.add_parser(
        'switch_mux', help="Switch the analog output mux to the requested "
                           "HICANN (counted from top)")
    command_set_voltage.add_argument(
        'hicann', type=int, help="HICANN counted from top")
    command_on = subparsers.add_parser(
        'on', help="Power the iboard up and set default voltages and the "
                   "analog mux to the top HICANN")
    command_off = subparsers.add_parser(
        'off', help="Power the iboard down and set voltages to zero disable "
                    "analog mux")

    args = parser.parse_args()

    hid = pyhid.pyhidaccess()
    hid.openHID(vid=0x0451, pid=0x4253)
    pyhid_cube = pyhid_cube.pyhid_cube(hid)
    print "Connected to CUBE:", pyhid_cube.readFirmwareVersion()
    iboard = iboard_ctrl(args.iboard, pyhid_cube)
    print "Configuring iboard {}".format(args.iboard)

    if args.command == 'on':
        iboard.set_defaults()
        print "All voltages have been set to default"
    elif args.command == 'off':
        iboard.power_down()
        print "All voltages have been set to 0V"
    elif args.command == 'set_voltage':
        iboard.set_voltage(args.voltage, args.value)
        print "Voltage {} set to {}V".format(args.voltage, args.value)
    elif args.command == 'switch_mux':
        iboard.switch_mux(7 - args.hicann)
        print "Analog output muxes set to HICANN {} (counted from top)".format(
            args.hicann)
    else:
        parser.usage()
