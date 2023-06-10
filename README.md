<h1 align="center">
  <a href="https://github.com/Ahmed/NeTrainSim">
    <img src="https://github.com/AhmedAredah/NeTrainSim/assets/77444744/d38d4ebe-3eb6-4877-b168-032373a54b11" alt="NeTrainSim"/>
  </a>
  <br/>
  NeTrainSim [Network Trains Simulator]
</h1>

<p align="center">
  <a href="http://dx.doi.org/10.2139/ssrn.4377164">
    <img src="https://zenodo.org/badge/DOI/10.2139/ssrn.4377164.svg" alt="DOI">
  </a>
  <a href="https://www.gnu.org/licenses/gpl-3.0">
    <img src="https://img.shields.io/badge/License-GPLv3-blue.svg" alt="License: GNU GPL v3">
  </a>
    <a href="">
    <img src="https://img.shields.io/badge/CLA-CLA%20Required-red" alt="CLA Required">
  </a>
  <a href="https://github.com/AhmedAredah/NeTrainSim/releases">
    <img alt="GitHub tag (latest by date)" src="https://img.shields.io/github/v/tag/AhmedAredah/NeTrainSim.svg?label=latest">
  </a>
  <img alt="GitHub All Releases" src="https://img.shields.io/github/downloads/AhmedAredah/NeTrainSim/total.svg">
</p>

<p align="center">
  <a href="https://ahmedaredah.github.io/NeTrainSim/" target="_blank">Documentation</a> |
  <a href=https://join.slack.com/t/netrainsim/shared_invite/zt-1x0esp2x0-8a9TeK14JbIqcLl_PkyZMA" target="_blank">Slack</a> |
  <a href="##Contributing">Contributing</a>
</p>

# Network Train Simulator (NeTrainSim)

NeTrainSim is an open-source train simulation software that allows users to simulate and analyse train operations on a large rail network. It is designed to be flexible, modular, and easy to use. It is built using Qt6 under the [GNU General Public License (GPL) version 3](https://www.gnu.org/licenses/gpl-3.0.en.html).

## How to Cite

```bibtex
@inproceedings{aredah2023netrainsim,
  title={NeTrainSim: A Network Freight Train Simulator for Estimating Energy/Fuel Consumption},
  author={Aredah, Ahmed and Fadhloun, Karim and Rakha, Hesham and List, George},
  booktitle={102nd Transportation Research Board Annual Meeting},
  year={2023}
}
```


## Features

- **Accessible and affordable**: NeTrainSim is freely accessible to anyone with internet and laptop access, without the need for costly commercial licenses.
    
- **Open-source development**: NeTrainSim is an open-source project, allowing researchers to modify the code and collaborate easily.
    
- **Advanced network modeling**: NeTrainSim can model entire rail networks, including country-scale simulations.
    
- **Versatile and adaptable**: NeTrainSim can be adapted to various rail network types and configurations, providing flexibility in simulations.
    
- **Energy-efficient**: NeTrainSim's Energy Consumption module supports a wide range of train technologies, accommodating future advancements.
    
- **Second-by-second data tracking**: NeTrainSim provides detailed data, including energy consumption, on a second-by-second basis for each train.
    
- **Advanced train dynamics**: NeTrainSim uses advanced train dynamics models, including operator aggressiveness, for accurate energy consumption representation.
    
- **Energy optimization**: NeTrainSim includes an energy optimization module for optimizing individual or grouped trains (under development).


## Getting Started
To get started with NeTrainSim, you will need to download and install the software on your computer. You can do this by downloading the latest release version on the releases page.

### Prerequisites
There is no prerequisite required to run NeTrainSim as the installer has all the required 3rd party packages. 

### Installing
Once the file is downloaded on your hard drive. Double click the downloaded file and follow the instruction to install the application. The default installation folder is `C:\Program Files\NeTrainSim`; however, you can alter this installation path as you wish during the installation process. 

Note that this version of NeTrainSim does not have a GUI and it only allows access through the command line interface.

### Running

#### Shell interface 
1. Open a shell/command line window,

2. Navigate to the installation path using 

   ```shell
   cd "C:\Program Files\NeTrainSim"
   ```

   if you changed the default installation path, make sure to replace `C:\Program Files\NeTrainSim` with your installation path.

3. To request help from NeTrainSim, type the following in the shell

   ```shell
   NeTrainSim -h
   ```

4. NeTrainSim allows the following command flags. Flags with required values are necessary to run the simulator.  

   ```html
   -h, --help, -?                          Display this help message.
   -v, --version                           Displays version information.
   -n, --nodes <nodesFile>                 [Required] the nodes filename.
   -l, --links <linksFile>                 [Required] the links filename.
   -t, --trains <trainsFile>               [Required] the trains filename.
   -o, --output <outputLocation>           [Optional] the output folder address.
                                          Default is
                                          'C:\Users\<USERNAME>\Documents\NeTrain
                                          Sim\'.
   -s, --summary <summaryFilename>         [Optional] the summary filename.
                                          Default is
                                          'trainSummary_timeStamp.txt'.
   -a, --all <summarizeAllTrains>          [Optional] bool to show summary of
                                          all trains in the summary file.
                                          Default is 'false'.
   -e, --export <exportTrajectoryOptions>  [Optional] bool to export
                                          instantaneous trajectory.
                                          Default is 'false'.
   -i, --insta <instaTrajectoryFile>       [Optional] the instantaneous
                                          trajectory filename.
                                          Default is
                                          'trainTrajectory_timeStamp.csv'.
   -p, --timeStep <simulatorTimeStep>      [Optional] the simulator time step.
                                          Default is '1.0'.
   ```

   Example of a minimum-flag command

   ```shell
   NeTrainSim.exe -n "path\to\nodes\file" -l "path\to\links\file" -t "path\to\trains\file"
   ```

   In this case, the default output Location is `C:\Users\<USERNAME>\Documents\NeTrainSim`  where `<USERNAME>` should be replaced with the current session user name.
                                                                                          
## Collaborators
The development of NeTrainSim has been a collaborative effort. The following individuals have contributed to the development and maintenance of the simulator:
                                                                                          
- **Ahmed Aredah, M.Sc.**: 
     - Ph.D. student, Dept. of Civil and Environmental Engineering, Virginia Tech  
     - M.Sc. Student, Dept. of Computer Science | Engineering, Virginia Tech  
     - Graduate Research Assistant at Virginia Tech Transportation Institute

- **Karim Fadhloun, Ph.D.**:
     - Research Associate at Virginia Tech Transportation Institute

- **Hesham A. Rakha, Ph.D. P.Eng., F.IEEE**: 
     - Samuel Reynolds Pritchard Professor of Engineering, Charles E. Via, Jr. Dept. of Civil and Environmental Engineering
     - Courtesy Professor, Bradley Department of Electrical and Computer Engineering
     - Director, Center for Sustainable Mobility at the Virginia Tech Transportation Institute
     - Fellow of Asia Pacific Artificial Intelligence Association
     - Fellow of the American Society of Civil Engineers
     - Fellow of the Canadian Academy of Engineering
     - Fellow of IEEE

## Contributing

If you are interested in contributing to NeTrainSim, please read the CONTRIBUTING.md file for more information on how to get started.


## License

This program is distributed under the terms of the [GNU General Public License (GPL) version 3](https://www.gnu.org/licenses/gpl-3.0.en.html). Please see the `LICENSE` file for more information.
