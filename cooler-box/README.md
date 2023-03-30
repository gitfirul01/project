Keterangan: 

    Deadline 25 Juli
    v1.0


Problem:
   
    Button pada slave (MEGA)
    

Spek upgrade:

[ ] Ada handle disamping kanan kiri utk mengangkat alat tsb
    ==> minta bantuan mekanik
    
[ ] Ada remote pengatur suhu dan on off

    Step:
    1. konfigurasi master           [X]
    2. konfigurasi slave            [X]
    3. desain remot                 [ ]
    
    Ket:
    pakai dua modul bluetooth
    master : arduino nano
    slave : arduino mega


[X] Ada alarm jika suhu tdk sesuai

    Step:
    1. akses DS18B20                            [X]                           
    2. setting batas suhu                       [X]
    3. tes alarm                                [X]
    
    Ket:
    sensor suhu : DS18B20
    nilai ambang batas suhu bisa diatur
    toleransi +-1 derajat celcius
    buzzer


[X] Ada monitor jumlah kantong yg disimpan didalam 

    Step:
    1. desain rangkaian                   [X]
    2. pemasangan switch dengan kabel     [X]
    3. pemasangan switch pada rak         [X]
     . memperbesar lubang pada box        [X]
    4. pemasangan rangka                  [X]
    
    Ket:
    Rak-rak di dalam ada nomornya
    pakai switch push button >> pullup, normally high


[ ] Display: jumlah kantong di dalam, suhu, jam

    Step
     . konfigurasi switch kantong         [X]
     . konfigurasi sensor suhu            [X]
     . konfigurasi RTC                    [X]
    1. desain tampilan                    [X]
    2. coding tampilan                    [X]
    3. menghubungkan arduino ke display   [ ]
    
    Ket:
    modul jam pakai RTC
    display : LCD Nextion 7"
    dibawah LCD dikasih push button sebagai pengganti remote
