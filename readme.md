## 8th semester Cloud Computing Project. <br>
## Topic: implementation of lamports Mutual exclusion using logical clock. <br>
**Contributors:** <br>
| Name                   | Roll Number | Email                    |
|------------------------|-------------|--------------------------|
| Amit Kumar Pandit      | 20CS01035   | [20cs01035@iitbbs.ac.in](mailto:20cs01035@iitbbs.ac.in)  |
| Gond Shubham Surendra | 20CS01062   | [20cs01062@iitbbs.ac.in](mailto:20cs01062@iitbbs.ac.in)  |
| Tapas Ranjan Nayak    | 20CS01064   | [20cs01064@iitbbs.ac.in](mailto:20cs01064@iitbbs.ac.in)  |

## Approach

### Lamport's Mutual Exclusion Algorithm

Lamport's Mutual Exclusion Algorithm is a distributed algorithm used to ensure mutual exclusion among concurrent processes in a distributed system. The algorithm is based on Lamport's logical clocks and relies on message passing between processes to coordinate access to shared resources.

#### Key Concepts

1. **Logical Clocks**: Lamport's algorithm uses logical clocks to order events in a distributed system. Each process maintains its logical clock, and every message sent between processes includes the sender's logical clock value.

2. **Requesting Access**: When a process wants to access a critical section, it sends a request message to all other processes. The request message includes the requesting process's logical clock value.

3. **Releasing Access**: After completing its critical section execution, a process releases access by sending release messages to all other processes. These release messages inform other processes that the critical section is now available for use.

### Implementation Details

#### Socket Communication

- The system uses TCP/IP socket communication for message passing between processes running on different computers.
- Three sockets are created for bidirection communication between three computer i.e. one is between 1 and 2 another is between 2 and 3 and other is between 3 and 1.
- Messages are serialized into byte arrays before being sent over the network and deserialized upon receipt.

#### Threaded Communication

- Each process spawns two threads to handle incoming messages and sending message.
- Listener thread continuously listen for messages and handle them accordingly, updating the process's logical clock.
- Sender thread send for request for accessing critical section.

#### Critical Section Execution

- Processes access the critical section based on their position in the request queue.
- A process can access the critical section only when it is at the front of the queue and has received reply messages from all other processes.
- After executing the critical section, the process releases access by sending release messages to all other processes.
- Output.txt in computer1 store the file and computer2 and computer3 access the file as critical section.

## How to Run

To run the distributed system implemented with Lamport's Mutual Exclusion Algorithm, follow these steps:

### Prerequisites

1. **Computers**: You need three computers connected to the same network.
2. **Compiler**: Each computer must have a C++ compiler installed (e.g., g++).
3. **Code**: Download the following files and place them on their respective computers:
    - `computer1.cpp`: Place on the first computer.
    - `computer2.cpp`: Place on the second computer.
    - `computer3.cpp`: Place on the third computer.

### Configuration

1. **IP Addresses and Ports**:
    - Open each source file (`computer1.cpp`, `computer2.cpp`, `computer3.cpp`) and update the following variables according to your network configuration:
        - `SERVER_IP`: IP address of the computer where the server is running.
        - `PORT1`, `PORT2`, `PORT3`: Port numbers for communication and file access. Ensure they are not being used by other applications.

### Compilation

1. **Compile Source Files**:
    - On each computer, open a terminal and navigate to the directory containing the source file.
    - Compile the source file using the C++ compiler. For example:
        ```
        g++ -o computer1 computer1.cpp -pthread
        ```

### Execution

1. **Run Computers Sequentially**:
    - Start the computers in the following sequence:
        1. First Computer: Run `computer1`.
        2. Second Computer: Run `computer2`.
        3. Third Computer: Run `computer3`.
    - Ensure that each computer is running before proceeding to the next one.

### Output

1. **Critical Section Output**:
    - Once all computers are running, the critical section execution will begin.
    - The output of the critical section execution will be written to the file `output.txt` on the computer1.
    - Check the contents of `output.txt` on the first computer to view the results of the critical section execution.

### Termination

1. **Exiting the Program**:
    - To stop the program, terminate each running process individually by closing the terminal windows or using the appropriate termination command.



