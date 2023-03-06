# Network Train Simulator (NeTrainSim)

NeTrainSim is an open-source train simulation software that allows users to simulate and analyze train operations on a large rail network. It is designed to be flexible, modular, and easy to use.

## How to Cite

```bibtex
@inproceedings{aredah2023netrainsim,
  title={NeTrainSim: A Network Freight Train Simulator for Estimating Energy/Fuel Consumption},
  author={Aredah, Ahmed and Fadhloun, Karim and Rakha, Hesham and List, George},
  booktitle={102nd Transportation Research Board Annual Meeting},
  year={2023}
}
```


## Getting Started
To get started with NeTrainSim, you will need to download and install the software on your computer. You can do this by downloading the latest release version on the releases page.

### Prerequisites
There is no prerequisite required to run NeTrainSim as the installer has all the required 3rd party packages. 

### Installing
Once the file is downloaded on your hard drive. Double click the downloaded file and follow the instruction to install the application. The default installation folder is "C:\Program Files\NeTrainSim"; however, you can alter this installation path as you wish during the installation process. 

Note that this version of NeTrainSim does not have a GUI and it only allows access through the command line interface.

### Running

1. Open a shell/command line window,

2. Navigate to the installation path using 

   ```shell
   cd "C:\Program Files\NeTrainSim"
   ```

   if you changed the default installation path, make sure to replace C:\Program Files\NeTrainSim with your installation path.

3. To request help from NeTrainSim, type the following in the shell

   ```shell
   NeTrainSim -h
   ```

4. NeTrainSim allows the following command flags. Flags with required values are necessary to run the simulator.  

   ```html
   -n, --nodes <nodesFile>                 [Required] the nodes filename.
   -l, --links <linksFile>                 [Required] the links filename.
   -t, --trains <trainsFile>               [Required] the trains filename.
   -o, --output <outputLocation>           [Optional] the output folder address. Default 
	                                   		is C:\Users\<USERNAME>\Documents\NeTrainSim
   -s, --summary <summaryFilename>         [Optional] the summary filename. Default
	                                   		is trainSummary_timestamp.txt
   -e, --export <exportTrajectoryOptions>  [Optional] true to export instantaneous 
	                                   		trajectory, false otherwise. Default is false.
   -i, --insta <instaTrajectoryFile>       [Optional] the instantaneous trajectory 
	                                   		filename. Default is trainTrajectory_timestamp.csv
   ```

   Example of a minimum-flag command

   ```shell
   NeTrainSim -n "path\to\nodes\file" -l "path\to\links\file" -t "path\to\trains\file"
   ```

   In this case, the default output Location is "C:\Users\\<USERNAME>\Documents\NeTrainSim"  where \<USERNAME> should be replaced with the current session user name.

## Contributing

If you are interested in contributing to NeTrainSim, please read the CONTRIBUTING.md file for more information on how to get started.

## License

NeTrainSim is licensed under the MIT License. See the LICENSE file for more information.
