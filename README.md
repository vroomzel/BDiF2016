# BDiF2016

Assignment A

1. Build executable:
  make all
2. Specify path to input/output files in params.txt
3. (Optional) Adjust memory usage in params.txt by changing  -io_memory parameter (in bytes). Set it to 1/3 of total available memory that this program is allowed to use.
4. Start an mpi run:
  mpirun -n 16 main.x params.txt
