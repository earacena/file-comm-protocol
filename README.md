# file-comm-protocol

### Author

Emanuel Aracena Beriguete

### Description

A protocol designed for file transfer communications between server(s)/client(s).

### Usage

#### Clone the repo

```
git clone www.github.com/earacena/file-comm-protocol.git
```

Make a folder inside cloned directory called "logs", this is where packet details
will be stored. Skipping this step may result in some errors appearing on screen
but should not affect functionality.

#### Build

While inside the main directory of the project, type:

```
make
```

#### Running
Open two terminals, in terminal one run:
```
./netnode server . 4444 1
```

In terminal two, run:
```
./netnode client 127.0.0.1 4444 1
```

Logs with packet information are stored in ./logs folder and can be cleared with:
```
make clear
```
