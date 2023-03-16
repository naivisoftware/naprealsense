<br>
<p align="center">
  <img width=384 src="https://download.nap.tech/identity/svg/logos/nap_logo_blue.svg">
</p>

# Description

Intel [realsense](https://www.intelrealsense.com/) depth camera integration for NAP

## Installation
Compatible with NAP 0.6 and higher - [package release](https://github.com/napframework/nap/releases) and [source](https://github.com/napframework/nap) context.

### From ZIP

[Download](https://github.com/naivisoftware/naprealsense/archive/refs/heads/main.zip) the module as .zip archive and install it into the nap `modules` directory:
```
cd tools
./install_module.sh ~/Downloads/naprealsense-main.zip
```

### From Repository

Clone the repository and setup the module in the nap `modules` directory.

```
cd modules
clone https://github.com/naivisoftware/naprealsense.git
./../tools/setup_module.sh naprealsense
```

## Demo

Renders a pointcloud that is deformed using the `PointCloud` shader of the `naprealsense` module. 
The PointCloud shader implements the de-projection methods from the realsense SDK, deforming the point cloud mesh completely on the GPU.