# ctbmodules - DUNE DAQ module to control and read out the CTB hardware

Ported from original implementation in redmine:

<https://cdcvs.fnal.gov/redmine/projects/dune-artdaq/repository/revisions/develop/show/dune-artdaq/Generators/pennBoard>

<https://cdcvs.fnal.gov/redmine/projects/dune-artdaq/repository/revisions/develop/entry/dune-artdaq/Generators/TriggerBoardReader_generator.cc>

<https://cdcvs.fnal.gov/redmine/projects/dune-artdaq/repository/revisions/develop/entry/dune-artdaq/Generators/TriggerBoardReader.hh>

The package is in early development, and is targetted to be feature complete for integration into release 3.2.

## Instructions to Configure and Run with Nanorc

A basic default configuration for the CTB is available in the package. To configure for nanorc, navigate to the 'scripts' folder and run the following:

<code>
.ctbcontrollerconf_gen.py <confName>
</code>

Then invoke nanorc as recommended for the given release, for example (in 3.1):
<code>
nanorc <confName> <partitionName> boot conf start_run 101 wait 60 stop_run scrap terminate
</code>
