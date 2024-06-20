int april2023zadatak2A(int argc, char* argv[])
{
    // Napisati MPI program koji realizuje množenje matrice A dimenzija n x n i vektora B dimenzija n, čime se dobija
    // rezultujući vektor C dimenzija n, Matrica A i vektor B se inicijalizuju u master procesu. Broj procesa je p i 
    // uređeni su kao matrica q x q (q ^ 2 = p). Matrica A je podeljena u blokove i master proces distribuira 
    // odgovarajuće blokove matrice A po procesima kao što je prikazano na slici 1. za n = 8 i p = 16. Vektor b je 
    // distribuiran po procesima tako da proces Pi dobija elemente sa indeksima i % q, i % q + q, i % q + 2q, ......,
    // i % q + n - q. Predvideti da se slanje vrednosti bloka matrice A svakom procesu obavlja odjednom. Svaki proces
    // (uključujući i master proces) obavlja odgovarajuća izračunavanja i učestvuje u generisanju rezultata koji se 
    // prikazuje u procesu sadrži minimum svih vrednosti u matrici A. Predvideti da se slanje blokova matrice A svakom 
    // procesu obavlja sa po jednom naredbom MPI_Send kojom se šalje samo jedan izvedeni tip podatka. Slanje blokova 
    // vektora B i generisanje rezultata impelementirati korišćenjem grupnih operacija i funkcija za kreiranje novih
    // komunikatora.

    return 0;
}

int april2023zadatak2B(int argc, char* argv[])
{
    // Napisati MPI program koji kreira komunikator koji se sastoji od procesa na dijagonali u kvadratnoj mreži procesa.
    // Iz master procesa novog komunikatora poslati poruku svim ostalim procesima. Svaki proces novog komunikatora treba
    // da prikaže primljenu poruku.

    return 0;
}

int april2023zadatak3(int argc, char* argv[])
{
    // Napisati MPI Program koji manipuliše velikom količinom log informacija, tako što vrši paralelni upis i čitanje
    // binarne log datoteke. Log podaci nalaze se u datoteci file1.dat. Svi podaci vrše čitanje iste količine podataka,
    // tako da prvi proces čita podatke sa početka fajla, zatim drugi proces one u nastavku, itd. Upravo pročitane
    // podatke upisati u dve različite datoteke, prema sledećim zahtevima:
    // a) U datoteku file2.dat upisati podatke tako da se redosled upisa procesa ne može unapred predvideti.
    // b) U datoteku file3.dat upisati podatke tako da procesi, u redosledu od poslednjeg do prvog, upisuju 1/10 svojih
    // podataka, po round-robin principu.

    return 0;
}