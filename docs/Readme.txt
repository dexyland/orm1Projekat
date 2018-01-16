#################################################################################
#                                                                               #
#     Predmet:             Osnovi racunarskih mreza 1                           #
#     Projektni zadatak:   GRID - Management protocol                           #
#     Autori:              Stefan Jovanovic i Dejan Martinov                    #
#                                                                               #
#################################################################################


                - Uputstvo za prevodjenje i pokretanje programa -


    Za pravilno funkcionisanje programa potrebno je da istovremeno budu pokrenuti 
i  'server' i 'client'  programi. Potrebno  je otvoriti  dva terminala, jedan  za 
prevodjenje i pokretanje 'server' aplikacije, a drugi za prevodjenje i pokretanje 
'client'  aplikacije. Za  prevodjenje  i pokretanje  obe  aplikacije  potrebno je 
pozicionirati se u folder 'src'.

    Prevodjenje "server" aplikacije se  izvrsava  pomocu  "make server"  komande,
dok  se  prevodjenje  "client"  aplikacije izvrsava pomocu "make client" komande.
Ukoliko vec postoje izvrsne  datoteke, a doslo  je do  izmenu u  kodu,  neophodno
je  izvrsiti  komande  "make server_clean"  odnosno  "make client_clean"  kako bi
se te datoteke obrisale pre prevodjenja obe aplikacije.

    Pokretanje "server" aplikacije  se  izvrsava  pomocu "./server a d"  komande,
gde  su a i d  argumenti koji  predstavljaju  broj  analognih  odnosno digitalnih 
modula. Pokretanje  "server"  aplikacije  nije  moguce bez  navodjenja  pomenutih 
argumenata. Pokretanje "client" aplikacije se izvrsava pomocu "./client" komande.
Pre  pokretanja "client" aplikacije  potrebno je da "server" aplikacije  vec bude
pokrenuta.