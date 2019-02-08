Drive GIMAPI (the SMP query API) from the USS command line. The output is vertical-bar delimited, intended for consumption by scripts.

The documentation on this interface is in [SMP/E for z/OS Reference](https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/toc.htm). The most useful parts are:

* [Filter paramter syntax](https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/filter.htm)
* [Valid entry types](https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/entry.htm)
* [Valid subentry types](https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/subent.htm)
* [Example of QUERY command](https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/qsamp.htm)

# Some sample commands

* What are all the target and dlib zones?
```
smp -e targetzone,dlibzone -s srel
```
* What are all the SYSMODs APPLYed to an FMID?
```
smp -z targa -f "fmid='HDL1202'&apply='YES'" -e SYSMOD -s ename
```
* In what zones is a SYSMOD APPLYed?
```
./smp -e sysmod -s apply -f "ENAME='UI40054'"
```
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

# Usage message

```
usage: smp 
  -h    get usage help 
  -v    verbose 
  -d    debug 
  -m    display messages, even if return code is 0 or 4 
  -H    produce a header line in the output 
  -c <CSI>  DSN of CSI to use, fully qualified, no quoting required, lower case OK 
     default: "SMPE.ZOSV203.GLOBAL.CSI" 
     You can also use an abbreviation of 2.1, 2.2 or 2.3 to get
     the appropriate Rocket default CSI for that release.
  -z <zone(s)>  Zone selection: 
     global - Use the global zone 
     alltzones - Use all target zones 
     alldzones - Use all DLIB zones 
     * - Use all zones 
     zone[,zone] - Use specific names zone(s) 
     default: "*" 
  -e <entry type(s)>  Type(s) of entries to be displayed, such as: 
     assem, dddef, dlib, element, lmod, mac, sysmod, etc. 
     * - all entry types 
     default: "*" 
     See: https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/entry.htm 
  -s <subtype(s)>  Subtypes() of entries to be displayed 
     * - all subtypes types 
     default: "*" 
     See: https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/subent.htm 
  -f <filter>  Filter expression 
     See: https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/filter.htm 
     default: "" 
  -q <query>  This is just an alias for -f 
 
 If environment variable SMPCSI is set, it provides the default CSI; 
 otherwise "SMPE.ZOSV203.GLOBAL.CSI" is used. 
```