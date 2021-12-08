# mc714-A2

### Compilação e execução:

###### Algoritmo - Relógio Lógico de Lamport
  ```
  mpic++ lamport.cpp -o lamport
  ./lamport
  ```
###### Algoritmo - Exclusão mútua
  ```
  mpicc mutual.c -o mutual
  mpirun -n 3 --hostfile ../.mpi_hostfile mutual
  ```
###### Algoritmo - Eleição de líder
  ```
  mpic++ election.cpp -o election
  mpirun -n 3 --hostfile ../.mpi_hostfile election
  ```
