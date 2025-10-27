[root@webOSNano-unofficial ~]# free -h                                                                                                                                      
               total        used        free      shared  buff/cache   available                                                                                            
Mem:           912Mi       456Mi        54Mi        26Mi       402Mi       398Mi                                                                                            
Swap:          599Mi       9.0Mi       590Mi                                                                                                                                
[root@webOSNano-unofficial ~]# cat /proc/meminfo | grep -E "MemTotal|MemAvailable"                                                                                          
MemTotal:         934812 kB                                                                                                                                                 
MemAvailable:     407636 kB     
