Projetk zaliczeniowy PSiW
autor: Jakub Doman

This project is meant to use system functions of POSIX in c language to simulate shipping company and it's storage as well as couriers.


Compilation: 
 To compile this project all you need to do is type in "make" in your terminal from the Storagefacility_psiw directory

Instructions:
To start simulation
 - You ought to start by running 
    ./dyspozytornia <klucz> <liczba zamowien> <max_A_per_zam> <max_B_per_zam> <max_C_per_zam>  
 - Then feel fre to run any amount of ./magazyn <conf file> <klucz> programs

 - MAKE SURE THAT BOTH <klucz>s are the same

 - conf files are supposed to be formated like in the m_guide.txt
EXAMPLE 
make
./dyspozytornia klucz 30 10 10 10 
./magazyn conf/m2_conf.txt klucz