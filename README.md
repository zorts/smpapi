Drive GIMAPI (the SMP query API) from the USS command line.

Some sample commands:

* To get a summary of an LMOD:
```
./smp -z targb -e lmod -f  "ENAME='CELHV003'"
```

* To find out what sysmod last updated the MODs in an LMOD:
```
   ./smp -z targb -s lastupd -e mod -f  "LMOD='CELHV003'"
```

* To find out what LMODs contain a MOD:
```
   ./smp -z targb -e mod -s lmod -f "ENAME='CELHABND'"
```
