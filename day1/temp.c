root@dq1:~# ./ble.sh -m "8A:88:4B:00:1E:ED"                                                             
TARGET_MAC: 8A:88:4B:00:1E:ED                                                                           
Current BLE MAC address is: 88:88:88:88:88:88                                                           
Is this the intended MAC address? (y/n):                                                                
y                                                                                                       
Proceeding with the intended MAC address.                                                               
[bluetooth]# scan ono bluetoothd...                                                                     
hci0 new_settings: powered bondable ssp br/edr secure-conn                                              
hci0 type 1 discovering on                                                                              
Agent registered                                                                                        
SetDiscoveryFilter success                                                                              
[CHG] Controller 88:88:88:88:88:88 Pairable: yes                                                        
Discovery started                                                                                       
[CHG] Controller 88:88:88:88:88:88 Discovering: yes                                                     
[bluetooth]#                                                                                            
                                                                                                        
[NEW] Device C8:8A:9A:F7:9C:97 GR-RD10-NC100QX                                                          
[bluetooth]# Device C8:8A:9A:F7:9C:97 GR-RD10-NC100QX                                                   
Device C8:8A:9A:F7:9C:97 GR-RD10-NC100QX                                                                
[NEW] Device 8A:88:4B:00:1E:ED minzzl-HP-Z6-G5-Workstation-Desktop-PC                                   
[bluetooth]# Device 8A:88:4B:00:1E:ED minzzl-HP-Z6-G5-Workstation-Desktop-PC                            
Device C8:8A:9A:F7:9C:97 GR-RD10-NC100QX                                                                
Found target MAC: 8A:88:4B:00:1E:ED                                                                     
                                                                                                        
Removing all devices...                                                                                 
[DEL] Device 8A:88:4B:00:1E:ED minzzl-HP-Z6-G5-Workstation-Desktop-PC                                   
Device has been removed                                                                                 
[DEL] Device C8:8A:9A:F7:9C:97 GR-RD10-NC100QX                                                          
Device has been removed    
