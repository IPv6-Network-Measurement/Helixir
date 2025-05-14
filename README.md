# Helixir

## I. Introduction

This repository hosts Helixir, a feedback-based efficient and high hit-rate IPv6 router interface discovery system. Helixir's core design includes a dynamic budget allocation mechanism across probing rounds, an inter-prefix budget allocation strategy that adequately trades off exploration and exploitation, and a Hop Limit selection method based on Thompson sampling. 

## II. Requirement

Before running Helixir, make sure the following dependencies are installed:

- GCC (for compiling source code)
- Bash (for running scripts)
- libpcap (for packet capture and network probing)
- Linux system with raw socket support (required for probing)

## III. Quick Start

### **Step 1: Data Preparation**

Before running the probing process, we need to generate the target prefix datasets. The following scripts are used to create two types of datasets:

```bash
bash Tools/generate_BGP_prefixes.sh
bash Tools/generate_Hitlist_prefixes.sh
```

#### **Dataset Descriptions:**

- **BGP Prefix Dataset**:
  - Extracted from BGP routing tables, this dataset contains IPv6 prefixes announced by Autonomous Systems (ASes).
- **Hitlist Prefix Dataset**:
  - Derived from known responsive IPv6 addresses, this dataset focuses on prefixes that historically contain active addresses. We aggregate and deduplicate these addresses to generate the final Hitlist prefix dataset.

### **Step 2: Compile Helixir**

Compiles the Helixir source code and generates the executable file `bin/main`.

```bash
make
```

### **Step 3: Set the Configuration and Run Helixir**

Edit `run.sh` to configure essential parameters for network probing. The following variables are included:

```bash
# Define parameters
INTERFACE_NAME=""
SOURCE_MAC=""
SOURCE_IP=""
GATEWAY_MAC=""
INPUT_FILENAME="input/Hitlist_prefixes.txt"
OUTPUT_FILENAME="$OUTPUT_DIR/$(date +'%Y%m%d_%H%M%S').log"
```

#### **Modifications and Purpose:**

| Parameter         | Description                                                  |
| ----------------- | ------------------------------------------------------------ |
| `INTERFACE_NAME`  | The network interface used for packet transmission.          |
| `SOURCE_MAC`      | The MAC address of the probing machine.                      |
| `SOURCE_IP`       | The IPv6 address of the probing machine.                     |
| `GATEWAY_MAC`     | The MAC address of the gateway router.                       |
| `INPUT_FILENAME`  | The file containing the target prefixes for probing.         |
| `OUTPUT_FILENAME` | A dynamically generated filename for logging results. The timestamp (`YYYYMMDD_HHMMSS`) ensures unique filenames across multiple runs. |

Once the dataset is prepared and the configuration is set, execute the probing process by running:

```bash
bash run.sh
```

This command initiates the probing process, transmitting packets and saving results to the specified log file.
