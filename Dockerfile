FROM ubuntu:20.10

WORKDIR /usr/src/recap

# Copy source files and directories
COPY . .

# Install required packages
RUN chmod +x ./install-packages.sh
RUN ./install-packages.sh 

# Create a build directory 
RUN mkdir build 

# Build the source using clang
WORKDIR /usr/src/recap/build
RUN cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++ ..
RUN make

# Run tests
RUN ./tests

# Return to root
WORKDIR /usr/src/recap

# Run the command line interface
ENTRYPOINT [ "./build/recap_cli" ]