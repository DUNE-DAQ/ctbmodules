# ctbmodules - DUNE DAQ module to control and read out the CTB hardware

Ported from original implementation in redmine:

<https://cdcvs.fnal.gov/redmine/projects/dune-artdaq/repository/revisions/develop/show/dune-artdaq/Generators/pennBoard>

<https://cdcvs.fnal.gov/redmine/projects/dune-artdaq/repository/revisions/develop/entry/dune-artdaq/Generators/TriggerBoardReader_generator.cc>

<https://cdcvs.fnal.gov/redmine/projects/dune-artdaq/repository/revisions/develop/entry/dune-artdaq/Generators/TriggerBoardReader.hh>


## Instructions to update the configuration and run with dunedaq v5 line

### Area setup
First of all you need a v5 area. 
To do this follow the instructions in the daqconf wiki, for example [fddaq-v5.3.2](https://github.com/DUNE-DAQ/daqconf/wiki/Setting-up-a-fddaq%E2%80%90v5.3.2-software-area). 

Locally, in the top area, you also need the [base configuration repository](https://gitlab.cern.ch/dune-daq/online/ehn1-daqconfigs).
Please note that the repo on gitlab are only accessible via ssh key, so please register one in the CERN gitlab. 
I recommend you also set in your area a `.netrc` file as in the `np04daq` home, remember to change login to your CERN username. 
After that you can simply
```bash
git clone ssh://git@gitlab.cern.ch:7999/dune-daq/online/ehn1-daqconfigs.git
```
Or alternatively
```bash
cpm-setup -b fddaq-v5.3.2 eh1-daqconfigs
```
The first one is a direct clone, while the second sets up the configuration repo to do some more advance operation, so the default branches might be a little strange. 
The second only works if you have setup the `.netrc` file correctly
To conclude, just 
```bash
source ehn1-daqconfigs/setup_db_path.sh 
```

### Update the ctb configuration


