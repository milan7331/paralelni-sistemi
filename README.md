# Paralelni sistemi
Uprađeni ispitni zadaci iz predmeta Paralelni sistemi  
Elektronski fakultet 2023/24


Ne garantujem da su zadaci 100% ispravni. Ukoliko uočite grešku ili želite nešto da dodate uradite pull request.

# Setup
## MPI
1. Instalirati msmpisetup i msmpisdk (naći na guglu majkrosoftov sajt...).
2. Instalirati C++ desktop development pack za visual studio (preko vs installera).
3. Kreirati C++ console projekat.
4. U glavni C++ dodati `#include "mpi.h"`.
5. Project -> Properties -> C/C++ -> additional include directiories -> dodati putanju gde je mpi sdk include folder (instalacija iz prvog koraka)
6. Project -> Properties -> Linker -> additional library directories -> dodati putanju do mpi sdk Lib\x64 foldera (obratiti pažnju da u configuration manageru samog projekta piše da pravimo x64 aplikaciju).
7. Project -> Properties -> Linker -> Input -> Additional dependencies -> ukucati msmpi.lib
8. Apply & Build projekat.
9. Otvoriti terminal i pozicionirati se u ./x64/debug odnosno ./x64/release folder. Komanda za pokretanje je `mpiexec -n 5 MPI-Zadaci.exe` Gde je n = 5 broj procesa koji želimo.

## OpenMP
1. Kreirati C++ projekat.
2. U glavni C++ fajl dodati `#include <omp.h>`.
3. Project -> Properties -> C/C++ -> Language -> Open MP Support promeniti na Yes.
4. Pokrenuti normalno kao i svaki drugi console projekat.

## CUDA