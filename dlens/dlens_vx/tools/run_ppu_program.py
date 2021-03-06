import ctypes
import argparse

from dlens_vx import halco
from dlens_vx.sta import PlaybackProgramBuilder, PlaybackProgramExecutor, \
    AutoConnection, DigitalInit, generate
from dlens_vx.hal import PPUControlRegister, Timer, PPUMemoryBlock
from dlens_vx.lola import PPUElfFile

_DEFAULT_PPU = halco.PPUOnDLS(0)


class PPUTimeoutError(Exception):
    pass


def load_and_start_program(executor: PlaybackProgramExecutor,
                           binary_path: str,
                           ppu: halco.PPUOnDLS = _DEFAULT_PPU) -> None:
    """
    Load a PPU program and trigger its execution.

    :param executor: Connected executor to be used for loading and starting the
                     program.
    :param binary_path: Path to the unstripped (*.bin) program to be loaded
    :param ppu: PPU the program is started on.
    """
    builder = PlaybackProgramBuilder()

    ppu_control_reg_run = PPUControlRegister()
    ppu_control_reg_run.inhibit_reset = True

    ppu_control_reg_reset = PPUControlRegister()
    ppu_control_reg_reset.inhibit_reset = False

    program_file = PPUElfFile(binary_path)
    program = program_file.read_program()
    program_on_ppu = halco.PPUMemoryBlockOnPPU(
        halco.PPUMemoryWordOnPPU(0),
        halco.PPUMemoryWordOnPPU(program.size() - 1)
    )

    program_on_dls = halco.PPUMemoryBlockOnDLS(program_on_ppu, ppu)

    # Ensure PPU is in reset state
    builder.write(ppu.toPPUControlRegisterOnDLS(), ppu_control_reg_reset)

    # Manually initialize memory where symbols will lie, issue #3477
    for _name, symbol in program_file.read_symbols().items():
        value = PPUMemoryBlock(symbol.coordinate.toPPUMemoryBlockSize())
        symbol_on_dls = halco.PPUMemoryBlockOnDLS(symbol.coordinate, ppu)
        builder.write(symbol_on_dls, value)

    # Write PPU program
    builder.write(program_on_dls, program)

    # Set PPU to run state, start execution
    builder.write(ppu.toPPUControlRegisterOnDLS(), ppu_control_reg_run)
    executor.run(builder.done())


def stop_program(executor: PlaybackProgramExecutor,
                 print_mailbox: bool = True,
                 ppu: halco.PPUOnDLS = _DEFAULT_PPU) -> int:
    """
    Stop the PPU and evaluate the exit code. Optionally, read back the mailbox
    and print it.

    :param executor: Connected executor to be used for stopping the program.
    :param print_mailbox: Read back and print the mailbox as string to stdout.
    :param ppu: PPU the program is stopped on.
    :return Exit code of the program
    """
    ppu_control_reg_end = PPUControlRegister()
    ppu_control_reg_end.inhibit_reset = False

    builder = PlaybackProgramBuilder()
    builder.write(ppu.toPPUControlRegisterOnDLS(), ppu_control_reg_end)
    return_handle = builder.read(
        halco.PPUMemoryWordOnDLS(halco.PPUMemoryWordOnPPU.return_code, ppu))

    if print_mailbox:
        mailbox_handle = builder.read(
            halco.PPUMemoryBlockOnDLS(halco.PPUMemoryBlockOnPPU.mailbox, ppu))
    else:
        mailbox_handle = None

    # Wait for all read responses to arrive
    builder.write(halco.TimerOnDLS(), Timer(0))
    builder.wait_until(halco.TimerOnDLS(), 1000)

    # Run builder
    executor.run(builder.done())

    # Print Mailbox
    if print_mailbox:
        print(mailbox_handle.get().to_string())

    # Return the exit code
    return ctypes.c_int32(int(return_handle.get().value)).value


def wait_until_ppu_finished(executor: PlaybackProgramExecutor,
                            timeout: int = None,
                            ppu: halco.PPUOnDLS = _DEFAULT_PPU) -> None:
    """
    Poll the PPU status register until program has finished. An optional
    timeout may be specified.

    :param executor: Connected executor to be used for polling the ppu status
                     register
    :param timeout: Timeout for the PPU program execution. Any number of
                    FPGA cycles may be given, which correspond to the
                    number of cycles the FPGA is actively waiting.
                    The time needed for communication with the FPGA is
                    neglected.
    :param ppu: PPU whose completion is awaited.
    :raises PPUTimeoutError: PPU did not finish within max_polls
    """
    per_poll_wait = 1000
    max_num_polls = timeout // per_poll_wait if timeout else None

    poll_builder = PlaybackProgramBuilder()
    status_handle = poll_builder.read(ppu.toPPUStatusRegisterOnDLS())
    poll_builder.write(halco.TimerOnDLS(), Timer(0))
    poll_builder.wait_until(halco.TimerOnDLS(), per_poll_wait)
    poll_program = poll_builder.done()

    executor.run(poll_program)
    num_polls = 1
    while status_handle.get().sleep is not True:
        executor.run(poll_program)

        if timeout is not None and num_polls > max_num_polls:
            raise PPUTimeoutError("PPU execution did not finish in time.")

        num_polls += 1


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Execute PPU programs")
    parser.add_argument('program', type=str)
    parser.add_argument(
        '--wait', type=int, default=int(1E7),
        help="Timeout for the PPU program execution. Any number of FPGA "
             "cycles may be given, which correspond to the number of cycles "
             "the FPGA is actively waiting. The time needed for communication "
             "with the FPGA is neglected.")
    parser.add_argument('--ppu_id', type=int, default=_DEFAULT_PPU.toEnum(),
                        help="PPU ID used for program execution.")
    args = parser.parse_args()

    with AutoConnection() as connection:
        init_builder, _ = generate(DigitalInit())
        connection.run(init_builder.done())

        load_and_start_program(connection, args.program,
                               halco.PPUOnDLS(args.ppu_id))
        wait_until_ppu_finished(connection, args.wait,
                                halco.PPUOnDLS(args.ppu_id))
        exit_code = stop_program(connection, ppu=halco.PPUOnDLS(args.ppu_id))

    if exit_code != 0:
        raise RuntimeError(f"PPU Program exited with exit code {exit_code}")
