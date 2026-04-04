# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall

# ==========================================
# make compile [FILE=main.cpp]
# ==========================================
# This target compiles the simulator. Default FILE is main.cpp.
FILE ?= main.cpp
compile:
	@echo "Compiling simulator with $(FILE):"
	$(CXX) $(CXXFLAGS) $(FILE) Processor.cpp ExecutionUnit.cpp LoadStoreQueue.cpp BranchPredictor.cpp -o main
	@echo "Build successful, 'main' created."

# ==========================================
# make run FILE=<filename.s>
# ==========================================
# Update this target to run whatever script or 
# program you wrote to preprocess the assembly labels. 
# Example below assumes a Python script named 'compiler.py'.
run:
	@echo "Preprocessing $(FILE)..."
	python3 compiler.py $(FILE)
	@echo "Preprocessing complete."

# ==========================================
# make test
# ==========================================
# This target runs the compiled simulator for all testcases
# and redirects their output to programs/outN.txt
test:
	@echo "Running all testcases..."
	@for i in 1 2 3 4 5; do \
		echo "Running programs/code$$i.txt -> programs/out$$i.txt"; \
		./main programs/code$$i.txt > programs/out$$i.txt; \
	done
	@echo "All tests complete. Outputs are in 'programs/' directory."

# ==========================================
# make clean
# ==========================================
clean:
	rm -f main
	rm -f programs/out*.txt