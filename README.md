<h1 align="center">
  <a href="https://github.com/VTTI-CSM/NeTrainSim">
    <img src="https://github.com/VTTI-CSM/NeTrainSim/assets/77444744/98776173-7574-404d-8eb4-bfdebe99f1cb" alt="NeTrainSim"/>
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
  <a href="https://github.com/VTTI-CSM/NeTrainSim/releases">
    <img alt="GitHub tag (latest by date)" src="https://img.shields.io/github/v/tag/VTTI-CSM/NeTrainSim.svg?label=latest">
  </a>
  <img alt="GitHub All Releases" src="https://img.shields.io/github/downloads/VTTI-CSM/NeTrainSim/total.svg">
  <a href="">
    <img src="https://img.shields.io/badge/CLA-CLA%20Required-red" alt="CLA Required">
    <a href="https://cla-assistant.io/VTTI-CSM/NeTrainSim"><img src="https://cla-assistant.io/readme/badge/VTTI-CSM/NeTrainSim" alt="CLA assistant" /></a>
  </a>
</p>

<div align="center">

<!-- ALL-CONTRIBUTORS-BADGE:START - Do not remove or modify this section -->
[![All Contributors](https://img.shields.io/badge/all_contributors-2-orange.svg?style=flat-square)](#contributors-)
<!-- ALL-CONTRIBUTORS-BADGE:END -->

</div>

<p align="center">
  <a href="https://github.com/VTTI-CSM/NeTrainSim/releases" target="_blank">Download NeTrainSim</a> |
  <a href="https://youtu.be/bbvnPn9zMsQ" target="_blank">How to Install and Quick Overview</a> |
  <a href="https://VTTI-CSM.github.io/NeTrainSim/" target="_blank">Documentation</a> |
  <a href="https://join.slack.com/t/netrainsim/shared_invite/zt-2913nksde-mmvbGCdz8k8GgjdfeQBXZQ" target="_blank">Slack</a>
</p>




# Network Train Simulator (NeTrainSim)

NeTrainSim is an open-source train simulation software that allows users to simulate and analyse train operations on a large rail network. It is designed to be flexible, modular, and easy to use. It is built using Qt6 under the [GNU General Public License (GPL) version 3](https://www.gnu.org/licenses/gpl-3.0.en.html).

## How to Cite

```bibtex
@article{aredah2024netrainsim,
  title={NeTrainSim: a network-level simulator for modeling freight train longitudinal motion and energy consumption},
  author={Aredah, Ahmed S and Fadhloun, Karim and Rakha, Hesham A},
  journal={Railway Engineering Science},
  pages={1--19},
  year={2024},
  publisher={Springer},
  doi={https://doi.org/10.1007/s40534-024-00331-x}
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

<p align = 'center'>
<img src="https://github.com/VTTI-CSM/NeTrainSim/assets/77444744/aaa0a970-84d8-435f-bcfc-dbf740546d76" width="500" alt ="main NeTrainSim window">
</p>

## Getting Started
To get started with NeTrainSim, you will need to download and install the software on your computer. You can do this by downloading the latest release version on the [releases page](https://github.com/VTTI-CSM/NeTrainSim/releases).

### Prerequisites
There is no prerequisite required to run NeTrainSim as the installer has all the required 3rd party packages. 

### Installing
Once the file is downloaded on your hard drive. Double click the downloaded file and follow the instruction to install the application. The default installation folder is `C:\Program Files\NeTrainSim`; however, you can alter this installation path as you wish during the installation process. 

Follow this video if you need a quick installation guidance: [https://youtu.be/bbvnPn9zMsQ](https://youtu.be/bbvnPn9zMsQ)


### Running

#### Using GUI interface
1. Please initiate the 'NeTrainSimGUI' application, which can be located either in the Windows Start menu or on your desktop.
2. Presented below is the primary user interface for the NeTrainSim application.

<p align = 'center'>
<img src="https://github.com/VTTI-CSM/NeTrainSim/assets/77444744/ffee71c7-972a-472a-b886-9fd128cefbf6" width="500" alt ="main NeTrainSim window">
</p>

3. For a comprehensive understanding of the application and its functionalities, please refer to the relevant documentation available within the 'Help' section of the menu.
                          
                          
#### Using shell interface 
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

## Contributors

<!-- ALL-CONTRIBUTORS-LIST:START - Do not remove or modify this section -->
<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<table>
  <tbody>
    <tr>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/heshamrakha"><img src="https://avatars.githubusercontent.com/u/11538915?v=4?s=100" width="100px;" alt="Hesham Rakha"/><br /><sub><b>Hesham Rakha</b></sub></a><br /><a href="#projectManagement-heshamrakha" title="Project Management">📆</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/AhmedAredah"><img src="https://avatars.githubusercontent.com/u/77444744?v=4?s=100" width="100px;" alt="Ahmed Aredah"/><br /><sub><b>Ahmed Aredah</b></sub></a><br /><a href="https://github.com/VTTI-CSM/NeTrainSim/commits?author=ahmedaredah" title="Code">💻</a></td>
    </tr>
  </tbody>
</table>

<!-- markdownlint-restore -->
<!-- prettier-ignore-end -->

<!-- ALL-CONTRIBUTORS-LIST:END -->
<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->

<!-- markdownlint-restore -->
<!-- prettier-ignore-end -->

<!-- ALL-CONTRIBUTORS-LIST:END -->

## Contributing

If you are interested in contributing to NeTrainSim, please read the CONTRIBUTING.md file for more information on how to get started.


## License

This program is distributed under the terms of the [GNU General Public License (GPL) version 3](https://www.gnu.org/licenses/gpl-3.0.en.html). Please see the `LICENSE` file for more information.

## Publications

Aredah, A.S., Fadhloun, K. & Rakha, H.A. (2024) NeTrainSim: a network-level simulator for modeling freight train longitudinal motion and energy consumption. Railw. Eng. Sci. https://doi.org/10.1007/s40534-024-00331-x

Aredah, A., Fadhloun, K., & Rakha, H. A. (2024). Energy optimization in freight train operations: Algorithmic development and testing. Applied Energy, 364, 123111. https://doi.org/10.1016/j.apenergy.2024.123111

Aredah, A., Du, J., Hegazi, M., List, G., & Rakha, H. A. (2024). Comparative analysis of alternative powertrain technologies in freight trains: A numerical examination towards sustainable rail transport. Applied Energy, 356, 122411. https://doi.org/10.1016/j.apenergy.2023.122411


