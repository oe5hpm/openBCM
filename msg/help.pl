\\HVERSION

Wersja tekstow pomocy:

Teksty pomocy dla skrzynki "BayCom-Mailbox" ver. 1.37t
Oryginal: OE3DZW z dn. 8.10.96 o godz. 16:45
Tlumaczenie: OE1KDA z dn. 5.11.96 o godz. 22:00
Uzupelnienia i modfikacje dla: "OpenBCM" ver. 1.07
wykonal: Janusz J. Przybylski SP1LOP (2004-2013)
Zyczenia i zazalenia prosze przesylac do: 
                                    SP1LOP@SR1BSZ.SZ.POL.EU
\\OUTLOOK_EXPRES

Serwer zapewnia pobieranie i wysylanie poczty przy pomocy standartowych
programow pocztowych np. Outlok Expres, Kurier Pocztowy SeaMonkey, i inne
Nalezy ustawic system wg ponizszych danych:

1. Poczta przychodzaca (POP3)      2. Poczta wychodzaca (SMTP)   
   serwer:  bbs.sr1bsz.ampr.org       serwer:  bbs.sr1bsz.ampr.org
     port:  8110                        port:  8025 

3. W ustawieniach zaawansowych:
   - Bez uwierzytelniania hasla
   - Serwer nie wymaga uwierzytelnienia
   - Serwer nie wymaga SSL

(Zamiast domeny mozna uzyc samego IP:  62.69.192.233)
                         Zycze powodzenia 73 de janusz SP1LOP
\\1-KROK

W tym miejscu moze byc opis jak i co nalezy wpisac dla ustawienia skrzynki
co i jak szukac. To bedzie tworzene na podstawie nadyslanych do mnie
zapytan tak wiec wszelkie problemy w korzystaniem prosze wysylac do:
SP1LOP
komenda:  s sp1lop   
          w tytule: podac jaki problem 
	  w tresci: podac w miare wyczerpujace zapyanie na jaki sie chce
	            uzyskac odpowiedz.

Mozna tez wysylac maile z zapytaniem na adres: 
          s HOWTO@SR1BSZ
          tytul: Jaki problem
          tresc: opisac szrzej o jaki problem chodzi

Na zadawane pytanie bedzie udzielal bezposredniej odpowiedzi oraz
umieszczal odpowiedz w katalogu HOWTO, oraz bedzie tworzyony ewentualnych
help w tym temacie dla innych, jak i czesc bedzie wykorzystana do opisu w
helpie 1-KROK.

Zalecam dla nowych uzytkownikow wpisanie komendy  help index i zostana
wyswietlone wszystkie dostepne aktualnie polecenia help. 
                                         73 de Sysop Janusz SP1LOP                 
Patrz tez: HELP ALTER, HELP ROZKAZY

\\#OK#

Zaczyna ciag dla przesylania pliku dwojkowego, bedzie zignorowany przez
BBS.

Zobacz tez HELP BINARY 

\\?=HELP

\\ACTIVE

"Aktywne Trasowanie" dawniej zostalo rozwiniete przez DL8HBS dla DPBox.
Ta instrukcja oblicza najlepsze trasowanie dla usermails spowodowanego
do trwale jakosciowe miary z wszystkim skierowanym ku przodowi bbs
partnera i spowodowany do informacja transmisji radiowej innych mailbox
ktore moga poslugiwac sie Aktywne Trasowanie.

p -sr      przedstawia wszystki znane aktywne trasowania adresow
wl <call>  jest widocznym streszczenie historii aktywny trasowania
           wartosci <znak> 

Zobacz tez  HELP WLOG i HELP PATH

\\A=ALTER

\\ADDLINUXSYS 

(sysop, init.bcm, tylko linux)
Syntaktyka: ADDLINUXSYSTEMUSER [ 0 | 1 ]
(Wartosc domyslna: 0)

Ta komende sysop "addlinuxsystemuser" moze ustawic w init.bcm 
wartosc 0 (= dezaktywuje) 
        1 (= aktywuje).

Jesli to jest ustawione jako 1, uzytkownik moze zdefiniowac jego haslo
uzytkownika dla skrzynki poczty elektronicznej system linux z ALTER
LINUXPW.
OpenBCM dodaje to haslo do linuxowego systemu plikow /etc/passwd i
/etc/shadow.
Jako loginshell "/bin/false" jest uzyty.
Usuwanie prawa uzytkowania od systemu linuxa nie jest mozliwe w tej
chwili od wewnatrz OpenBCM, dlatego sysop musi zrobic te plaszczyzne
pod systemem jesli to jest potrzebne.

Funkcja "addlinuxsystemuser" jest przydatna dla tych system BBS ktore
chca aby dodac jakis TCP/IP mozliwy dla ich praw uzytkowania 
(np. dla TCP/IP mail accounts) i czy sysop nie chca zainstalowac
ktoregokolwiek jakies inne specjalne radio dla pakietu TCP/IP srodki
programowe albo dodac TCP/IP linux system prawa uzytkowania recznie.

Poniewaz usershell jest ustawiony, w /bin/false, to jest zapisywac,
ze prawa uzytkowania staja sie dostepne do systemu ale nie dostawaj
prawdziwego rzeczywistego konta shell.

Zobacz tez HELP ALTER LINUXPW 

\\ADRESOWANIE

 Zaleznie od potrzeb nadane wiadomosci sa retransmitowane do dalszych
skrzynek sieci packet-radio. Po zakonczeniu retransmisji wiadomosci
prywatne sa kasowane, natomiast wiadomosci ogolne (biuletyny) pozostaja
we wszystkich skrzynkach na trasie retransmisji i ulegaja powieleniu.
Wiadomosci prywatne musza zawierac jednoznaczny adres skrzynki docelowej.
Bledny albo niekompletny adres opoznia lub uniemozliwia dostarczenie
poczty elektronicznej. 
W cyfrowej sieci amatorskiej stosowany jest nastepujacy system adresow
hierarchicznych:
   SR1BSZ.SZ.POL.EU
                 ^ kontynent, EU oznacza Europe
              ^ kraj, POL - Polska.
          ^ rejon (pole to nie jest konieczne)
     ^ znak wywolawczy skrzynki

Oznaczenia kontynentow:
AF   Afryka,    AS Azja,      AU Australia,    EU Europa,
MDLE Bliski Wschod,           NA Ameryka Pln., SA Ameryka Pld.

Oznaczenia krajow:
AUT Austria,     CHE Szwajcaria,  BEL Belgia,     DEU Niemcy,
DNK Dania,       FIN Finlandia,   FRA Francja,    HUN Wegry,
ITA Wlochy,      NLD Holandia,    POL Polska,     SWE Szwecja,   
SLO Slowenia,    SVK Slowacja,    TCH Czechy,     UKR Ukraina.

Wiadomosci dla wszystkich adresowane sa zbiorczo dla danego rejonu,
kontynentu lub calego swiata. Nalezy zwrocic szczegolna uwage na wybor
jezyka w zaleznosci od przewidywanego grona adresatow.
Przykladami adresowania sa:
POL   Polska,     DL   Niemcy,  OEDL   Austria i Niemcy,
OE    Austria     OK   Czechy,  EU     Europa,
WW    Swiat.

\\AFWDLIST 

(tylko sysop, _AUTOFWD opcje w config.h)
Syntaktyka: AF(WDLIST)

Ta komenda wygenerowuje plik "afwd.bcm" jesli "autofwdtime" nie jest
wartoscia 0. 
Aby automatycznie uaktualnic autorouter, "afwdlist" powinien zostac
wywolany okolo raz na dzien przez "crontab.bcm". 
AFWDLIST uzywa parametru AUTOFWDTIME.
Komplety AUTOFWDTIME numer dni, po ktorych stare marszruty sa zignorowane.
"AUTOFWDTIME 0" wylacza opcje autoforward.

Autoforward poczty jest okreslony w:
1.) sprawdzajac "fwd.bcm" jak zwykle, tylko, jesli zadna marszruta nie
    jest znaleziona, uzywaja nastepnego.
2.) sprawdzajac "afwd.bcm"
3.) patrzac w gore @ adres w H-adres datebase {ma sens tylko dla naprawde 
    nowy adresow, inaczej to powinno zostac znalezione w "afwd.bcm";
    jednakze to metoda jest calkiem szybka, wiec niema zadnej potrzeby
    wylaczac ja.

Zobacz tez HELP CRONTAB, HELP AUTOFWDT, HELP AUTOROUTER i HELP AUTOPATH 

\\AKTUALNOSCI=AKTUELL
\\AKTUELL

Rozkaz AKTUELL pozwala na odczytanie zbioru zawierajacego aktualnosci.
W przypadku dokonania zmian jest on nadawany automatycznie do korespon-
dentow.
\\ALTBOARDINFO

(sysop, init.bcm)
Syntaktyka: ALTBOARDINFO [ 0 | 1 ]
(wartosc domyslna: 0)

Jesli ALTBOARDINFO jest ustawiony na "1" boardinfo to "boardinf.bcm"
lacze jako boardname. Jesli ustawione na "0" to wyprowadzenie jest
zrobione w dwoch liniach.
\\ALTER

Rozkaz ALTER pozwala na zmiane niektorych, podanych ponizej parametrow
Kazdy z argumentow podawany jest w skrocie za pomoca pojedynczej litery.
Wlaczone sa opcje wymienione w rozkazie, ominiecie oznacza wylaczenie.
W przypadku zmiany jednego z argumentow mozliwe jest uzycie znaku + lub
minus odpowiednio w celu jego wlaczenia albo wylaczenia.
      a id +g

rozkaz       znaczenie                                          przyklady
------------+--------------------------------------------------+---------
ALTER
            Wywoluje liste parametrow.                         A

ALTER <znak>                                                   A OE1KDA
            wywoluje liste parametrow dla podanego znaku.

ALTER CHECK <arg>                                              A CH +Y
            Ustala dodatkowe argumenty dla rozkazu CHECK.
            Znaki + i - oznaczaja odpowiednio wybor lub
	                skasowanie pojedynczego parametru.

ALTER COMMAND <rozkaz>                                         A C D,D N
            Powoduje wykananie podanego rozkazu w momencie
	    polaczenia sie ze skrzynka. Mozliwe jest podanie
	    ciagu rozkazow oddzielonych przecinkami.
            Standardowo jest to rozkaz A C D 
	    (wyswietlenie spisu tresci wlasnej skrytki).  
	                                          Przyklad:    A C U,D
            Wyswietlenie spisu uzytkownikow i zawartosci
	    wlasnej skrytki.

ALTER DEFAULT                                                  A DEF
            Nadaje parametrom wartosci standartowe.

ALTER DELETE                                                   A DEL
            Sluzy do skasowania zestawu parametrow.

ALTER ECHO  0|1                                                A E 1
            Powoduje powtarzanie przez skrzynke otrzymanych
	    rozkazow w celu ulatwienia uzytkownikowi orientacji.
            Dotyczy to jedynie wejsc radiowych, a nie konsoli
	    skrzynki.

ALTER FORWARD [-L] <adres>                            A F SR1BSZ.SZ.POL.EU
            Ustala adres macierzystej skrzynki uzytkownika.
	    Adres ten komunikowany jest sasiednim skrzynkom
	    sieci (dotyczy to systemow BCM, DIEBOX-BBS i
	    DP-BBS.
            Parametr -L zapobiega zawiadamianiu pozostalych
	    skrzynek. 
            Adres skrzynki nie moze zawierac identyfikatora
	    SSID, w przypadkach gdy jest to niezbedne identy-
	    fikator nalezy podac lokalnie za pomoca rozkazu
	    A F -L  <znak>-<SSID>.
            Podawanie identyfikatora jest konieczne jedynie
	    w przypadku, gdy w sklad stacji wchodza skrzynki
	    roznych systemow (np. BCM i TCPIP).
            Wpisy starsze niz 6 miesiecy sa automatycznie
	    aktualizowane w momencie polaczenia uzytkownika
	    z jego skrzynka macierzysta.
            Sposob adresowania podany jest w opisie rozkazow:
            DIR PATH i FORWARD. Odpowiada rozkazowi MYBBS.

ALTER HELPLEVEL <poziom>                                       A H 1
            Sluzy do wyboru sposobu zglaszania sie skrzynki.
            Poziom 2 - Mniej doswiadczeni uzytkownicy powinni
                       korzystac z tego poziomu. Meldunek
		       gotowosci zawiera wowczas spis najwa-
		       zniejszych rozkazow.
            Poziom 1 - powoduje skrocenie tekstu powitalnego
	               (CTEXT).
            Poziom 0 - przewidziany dla doswiadczonych uzytko-
	               wnikow, nie zawiera spisu rozkazow.

ALTER HTTPSURFACE <poziom>                                     A HT 0
Konfiguruje look'n'feel powierzchni HTTP.

            Poziom 0 - uzyj ustawienie standardowe administra-
                       tora systemu ("defhttpsurface")
            Poziom 1 - aktywuje sie prosty wyglad z ramkami
                       ale bez support CSS 
            Poziom 2 - aktywuje sie mily wyglad bez ramek ale
                       z support CSS 
            Poziom 3 - aktywuje sie prosty wyglad bez ramek
                       i bez support CSS 

ALTER IDIR <arg>                                               A ID +Y
            Ustala dodatkowe argumenty dla rozkazu DIR dla
	    katalogow biuletynow ("info").
	                       Patrz takze: HELP ARGUMENTY

ALTER ILIST <arg>                                              A IL +Y
            Ustala dodatkowe argumenty dla rozkazu LIST dla
	    katalogow biuletynow ("info").
	                       Patrz takze: HELP ARGUMENTY

ALTER IREAD <arg>                                              A IR +Y
            Ustala dodatkowe argumenty dla rozkazu READ dla
	    katalogow biuletynow ("info").
	                       Patrz takze: HELP ARGUMENTY

ALTER LF <liczba>                                              A LF 0
            Ustala liczbe linii odstepu w tekstach nadawanych
	    przez skrzynke. Przemyslany wybor odstepow moze
	    zwiekszyc czytelnosc tekstu, jednak odbywa sie to
	    kosztem miejsca zajetego na ekranie.

ALTER LINES <#>                                                A L 20
            Ustala dlugosc wyswietlanej strony. Zakonczenie
	    strony jest sygnalizowane przez skrzynke. Wartosc
	    zero powoduje transmisje tekstu bez przerw.
            Patrz tez rozkaz: HELP STRONICOWANIE.

ALTER LOGINPWTY wybor rodzaju hasla dla interaktywnych lacznosci AX.25.

ALTER MYBBS  odpowiada ALTER FORWARD.

ALTER NAME <imie>                                              A N Janusz
            Podaje imie operatoraq. Jest ono umieszczane w
	    naglowkach wiadomosci.

ALTER NEWCALL <znak>                                           A NE sp0lop
            Wprowadzenie nowego znaku np. po zmianinach w
	    licencji.

ALTER NOPURGE                                                  A NOP
            Wylaczenie rozkazu PURGE dla wlasnego katalogu.
            Patrz tez: HELP PURGE CMDS.

ALTER NOTIFICATION [<znak>]                                    A NOT sp1kko
            Jesli czesto korzystac z klubowego znakw to wowczas
            wpisujesz ten znak i bedziesz mialem informacje,
	    jesli klub dostal nowego maila.

ALTER PROMPT <tekst>                                           A P (%b)-->
            Ustala znak zgloszenia skrzynki. W tekscie moga byc
            uzyte metasymbole. Patrz tez rozkaz HELP METASYMBOLE.
            Podany przyklad powoduje wyswietlanie zgloszenia w
	    stylu (OE1KDA)-->
            ALTER PROMPT bez parametrow powoduje uzycie standar-
	    dowego znaku zgloszenia.

ALTER PASSWORD [<tekst>|OFF]                                   A PW TO_JA
            Sluzy do wprowadzenie przez uzytkownika hasla
	    dostepu do skrzynki, jezeli funkcja ta jest wla-
	    czona przez operatora skrzynki. Tekst hasla
	    dodawany jest do ew. wczesniej wprowadzonego
	    hasla. Dlugosc hasla ograniczona jest do 39
	    znakow i nie moze ono zawierac srednikow i
	    przecinkow.
            Rodzaj hasla (algorytm generacji) wybierany jest
	    przez uzytkownika. 
	    W poprzednich wersjach oprogramowania stosowany
	    byl algorytm NETROM.
            Do skasowania hasla sluzy rozkaz A PW bez paramet-
	    row. Rozkaz A PW OFF powoduje wylaczenie funkcji
	    zabezpieczajacej. Moze ona byc nastepnie wlaczona
	    jedynie przez operatora skrzynki. Po wprowadzeniu
	    hasla adres skrzynki macierzystej moze byc zmieniany
	    jedynie w skrzynce pelniacej aktualnie ta funkcje.
            Patrz takze: HELP ALTER PWLINE, HELP UFWD HASLO
                         i HELP ALTER READLOCK.

A PWLINE 0 | 1                                                 A PWL 1
            Rozkaz umozliwia nadanie przez skrzynke na poczatku
            polaczenia linii o dowolnej tresci, np. "pw" albo
	    "sy".
            Pozwala to na automatyczne nadawanie hasla dostepu
	    przez rozne bedace w uzyciu programy terminalowe.
	    Parametr o wartosci 1 oznacza nadawanie linii, 
	    0 - jej brak.
            Patrz takze: HELP ALTER PW.

ALTER READLOCK <arg>                                           A READL 0
            Sluzy do ograniczenia dostepu do prywatnej poczty.
            Arg. 0 - pozwala wszystkim na odczyt wiadomosci
	             prywatnych.
            Arg. 1 - udostepnia tylko wiadomosci wlasne i
	             przeczytane przez adresata.
            Arg. 2 - udostepnia tylko wiadomosci wlasne.
                                   Patrz takze: HELP ALTER PW

ALTER REJECT <spis catalogue>                                 A R IBM C64
            Ustala liste katalogow pomijanych w wykazach wyswie-
	    tlanych za pomoca rozkazow DIR NEWS i CHECK.
	    Pomijane sa takze podkatalogi zawarte w wymienionych
	    katalogach glownych (dla rozkazu DIR NEWS).
	    Dodanie argumentu -R ogranicza wykazy tylko do
	    wymienionych katalogow.
            Liczba katalogow wylaczonych lub uwzglednianych jest
            ograniczona.      Patrz takze: HELP CHECK i HELP DIR

ALTER SFPWTYPW                                                A SFP haslo
            Wybor rodzaju hasla dla retransmisji wiadomosci.

ALTER SPEECH <prefiks kraju>                                   A S SP
            Sluzy do zmiany jezyka meldunkow skrzynki. Lista
            zainstalowanych jezykow wywolywana jest za pomoca
            rozkazu A S. W trakcie pierwszego polaczenia wybor
            jezyka dokonywany jest automatycznie na podstawie
            prefiksu.
            Przyklady:  A S PL   polski
                        A S GB   angielski
			A S XPL  polski - sysopa

ALTER STATUS <liczba>                                          A ST 0
            Sluzy do zmiany poziomu uprawnien uzytkownika:
            0 uprawnienia normalne, 1 bez ograniczonego pensum,
            2 dostep zablokowany.
                                 Patrz takze: HELP STATUS

ALTER TTYPW <haslo>                                     A TTYPW TO_TEZ_JA
            Sluzy do wprowadzenia hasla dostepu na lokalnym
	    wejsciu szeregowym (TTY) lub TELNET.
	    Dopuszczalna dlugosc hasla wynosi 7 znakow
	    alfanumerycznych, rozrozniane sa duze i male litery.

ALTER UDIR <arg>                                               A UD +Y
            Ustala dodatkowe argumenty dla rozkazu DIR dla
	    katalogow grupy prywatnej ("user-mail").
                                 Patrz takze: HELP ARGUMENTY

ALTER UFWD <znak_PBBS> <znak_digi> [<digi_wejsciowy>]
                                           A UFWD OE1KDA-8 SR1BSZ OE1XLR

            Pozwala na uruchomienie retransmisji poczty do
	    uzytkownika indywidualnego i podanie trasy pola-
	    czenia. W polu <znak_PBBS> - podawany jest znak
	    prywatnej skrzynki uzytkownika (z identyfikatorem
	    wtornym SSID), w pozostalych dwoch polach podawane
	    sa znaki wezlow na trasie polaczenia.
            Jako pierwszy podawany jest znak wezla najblizszego
            skrzynce sieci, a nastepnie - wezla w poblizu
	    uzytkownika.
            Jezeli np. stacja OE1KDA pragnie korzystac z
	    retransmisji wiadomosci naplywajacych do SR1BSZ-8,
	    a trasa polaczenia wyglada jak nastepuje:
            OpenBCM        wezel       wejscie       home_PBBS
            SR1BSZ-8 <---> SR1BSZ <---> oe1xlr <----> oe1kda-8
            Konieczne jest podanie rozkazu:
                             ALTER UFWD OE1KDA-8 SR1BSZ OE1XLR

ALTER ULIST <arg>                                              A UL +Y
            Ustala dodatkowe argumenty dla rozkazu LIST dla
	    katalogow grupy prywatnej ("user-mail").
                                Patrz takze: HELP ARGUMENTY

ALTER UREAD <arg>  ustala dodatkowe argumenty dla rozkazu      A UR +Y
            Ustala dodatkowe argumenty dla rozkazu READ dla
	    katalogow grupy prywatnej ("user-mail").
                                Patrz takze: HELP ARGUMENTY

Lista argumentow dla rozkazow Read/List/Dir wymieniona jest w opisie 
rozkazu HELP ARGUMENTY. Argumenty moga byc takze podane w wywolaniu
rozkazu.

\\ALTER.AWAY 

Syntaktyka: A(LTR) A(WAY) [ OFF | <tekst> ]

Ta komenda aktywje wiadomosc z daleka. Jest wskazane uzyc, by informowac
innego uzytkownika, ze wyjezdzasz. 
Jesli ALTER AWAY <tekst> jest zdefiniowany, nadawca wiadomosci do tego
uzytkownika dostaje automatyczna wiadomosc z skrzynki odbiorcy, ze
uzytkownik wybyl. <Tekst> jest dodany do tej wiadomosci.
<tekst> moze byc max. do 79 znakow.

Wskazanym aby dodac okres waznosci do <tekst>.

Przyklad:
         A A Jestem na wakacjach od 18.10.-23.10.2005. 73 Janusz 
         A AWAYE 23.10.2005

Wylaczenie funkcji: "A A OFF" lub "A A 0".

Jesli ty polaczysz sie ze swoja skrzynka po aktywowaniu tej funkcji,
to dostaniesz informacje, bys wylaczyl ta funkcje. 
Nie powinenes zapomniec jej wylaczyc.

Zobacz tez HELP ALTER.AWAYENDTIME  --> ustawienie czasu waznosci
informacji. 
\\ALTER.AWAYENDTIME 

Syntaktyka:  A(LTER) AWAYE(NDTIME) [ OFF | <date> ]

Ta komenda mozesz zdefiniowac date waznosci tekstu away, od kiedy funkcja
ma by automatyczna wylaczona. 
Ta komenda jest aktywna, jesli wczesniej uzyto komendy A A "<tekst>".

Zobacz tez: HELP ALTER.AWAY  --> ustawienie tresci informacji dla Away 

\\ALTER.BINMODE 

Syntaktyka: A(LTER) B(INMODE) [ AUTOBIN | YAPP | DIDADIT ]

Ta komenda wybiera zadany protokol transmisji dla pobierania binarnych
plikow. "ALTER BINMODE" przedstawia twoj biezacy rodzaj.

Zobacz tez HELP BIN, YAPP HELP i HELP DIDADIT 

\\ALTER.C=ALTER.COMMAND  
\\ALTER.CHECK

Przyklad: A CH +Y
Patrz: HELP ALTER i HELP ARGUMENTY.

\\ALTER.COMMAND

Przyklad: A C D,D N
Patrz: HELP ALTER.

\\ALTER.DEFAULT

Syntaktyka:  A(LTER) D(EFAULT)

Ustawia opcje dla wartosci standardowych.
Wszystkie osobiste ustawiania beda nieaktualne!
Przyklad: A DEF

\\ALTER.DELETE

(tylko sysop)
Syntaktyka:  A(LTER) DEL(ETE)

Wymazuje wszystkie twoje osobiste ustawiania calkowicie. Cofniecie tej
komendy nie jest mozliwe. Zapis uzytkownika zupelnie jest usuniety z
bazy danych prawa uzytkowania! W kontekscie, ALTER DEFAULT, ktorym zadne
znormalizowane wartosci nie beda ustawione.
 
\\ALTER.DEFFORMAT

Syntaktyka:  A(LTER) DIR(FORMAT) [ 0 | 1 ]

Data i format czasu DIR/LIST/READ wpisow teraz moga byc dostosowane.

Przyklady:
DIR 0  rodzony format:    31.05,  z Y opcje:  31.05.01 
DIR 1  formatu  ISO:     010531,  z Y opcje:  20010531 

Zobacz tez pomoc OPTION [CHECK | DIR | LIST | READ]
              i  HELP ALTER [C | ID | IL | IR | UD | UL | UR]

\\ALTER.ECHO

Syntaktyka:  A(LTER) E(CHO) [ 0 | 1 ]

Ta komenda mozesz kazac BBS, by wyslal kazda komenda, ktora wysylasz.
To nie przeszkadza sieci PR, poniewaz potwierdzenie bezposrednio jest
dodane na reakcji. Ale w ten sposob mozesz wyznaczyc komenda z jego
skutkiem latwo.
Produkcja potwierdzenia jest tylko aktywna, jesli interfejs radia pakietu
BCM jest uzywany (a nie console/file/modem).

\\ALTER.F=ALTER.FORWARD

\\ALTER.FBBCHECKMODE

Syntaktyka: A(LTER) FB(BCHECKMODE) [ 0 | 1 ]
(wartosc domyslna: 0)

Komenda Aktywuje/Dezaktywuje opcje FBBCHECK dla CHECK/READ. Jesli wartosc
jest ustawiona, na "1", opcja FBBCHECK jest aktywna, 
jesli zas na "0" wylaczona.
Jesli opcja FBBCHECK jest aktywna, poczta jest spisana kiedy robisz CHECK
z ciaglym numerem. Kiedy robiac "READ <nr>", gdzie <nr> jest numer listy
kontrolnej, rownowazna poczta jest pokazana.
To zachowanie jest takie same jako mailboxsystem FBB i bylo rekomendowane
od bardzo dawna przez uzytkownikow FBB. 
Wiecej komend FBB nie bedzie wprowadzonych w zycie do OpenBCM!

Patrz tez opcje w MAILLSERVER

\\ALTER.FD=ALTER.FDELAY

\\ALTER.FDELAY 

Syntaktyka: A(LTER) FD(ELAY) <min> 
(wartosc domyslna: 0)

Posiadana poczte wyslij do innego BBS natychmiast ale tylko po <min>
opoznienia.
Komenda zupelnie nie jest wprowadzona w zycie tym razem, byc moze poczta
bedzie forwardowana tylko po nastepnej komendzie POSTFWD.

\\ALTER.FHOLD=SETUSER.FHOLD 

\\ALTER.FORWARD

Syntaktyka:  A(LTER) F(ORWARD) [ [-L] <adres> ]
(Mozesz uzyc NH, MY(BBS) albo A(LTER) M(YBBS) zamiast)

Ustawienia twojego MYBBS. Cala poczta, ktora dostajesz bedzie wyslana do
tego adresu. Ta informacja MYBBS bedzie automatycznie wyslane do
wszystkich sasiadnich BCM i BBS Thebox. 
uzywajac opcji "-L" mozesz ustawic lokalny MYBBS (i MYBBS nie bedzie
wysylany).
Przyklad:  
     F DB0AAB.# BAY.DEU.UE  ustawia  MYBBS do DB0AAB.# BAY.DEU.EU 
     
Biezace adresy moga zostac zapisane w PATH DIR. 
Wiecej szczegolow komenda HELP FORWARD.

\\ALTER.HELPLEVEL

Syntaktyka:  A(LTER) H(ELPLEVEL) [ 0 | 1 | 2 ]

Ustawia helplevel. Nowi uzytkownicy powinni miec helplevel 2 z pelnymi
podpowiedziami programowymi. Plik "msg/cnew.<lang>" bedzie przedstawiony
dla uzytkownika. Uzycie ekspertow "0", w podpowiedzi jest wylaczony. 
Z helplevel "1" rozlegly tekst powitalny jest wylaczony, ale nie bedzie
pokazany gdy helplevel "2".

Przyklad:  A H 1    ustawia helplevel na 1
Patrz: HELP ALTER.

\\ALTER.HTTPSURFACE 

Syntaktyka: A(LTER) HT(TPSURFACE) [ 0 | 1 | 2 | 3 ]

Konfiguruje look'n'feel powierzchni HTTP.

 0 -> uzyj ustawienie standardowe administratora systemu ("defhttpsurface")
 1 -> aktywuje sie prosty wyglad z ramkami ale bez support CSS 
 2 -> aktywuje sie mily wyglad bez ramek ale z support CSS 
 3 -> aktywuje sie prosty wyglad bez ramek i bez support CSS 

Przyklad: a ht 0 

Zobacz też: HELP DEFHTTPSURFACE 

\\ALTER.IDIR 

Syntaktyka: A(LTER) ID(IR) [<opcja>]

Ta komenda wybierasz, ktora informacja bedzie pokazana, kiedy ty
listujesz biuletyny komenda DIR. Kazda opcja odpowiada jednemu
standardowemu formatowi.
Jesli ten standardowy format jest wyszczegolniony opcja jest ustawiona,
jesli standardowego formatu brak opcja nie jest ustawiona. Opcje moze
zostac ustawione/usuniete, jesli wpiszesz +/- z przodu tej opcji.
Przyklad:   ID +Y   dodajemozliwoscc wyboru Y (pokaz rok w dacie)

Patrz: HELP DIR OPTION i HELP ALTER DIRFORMAT 


\\ALTER.ILIST

Syntaktyka: A(LTER) IL(IST) [<opcja>]

Ta komenda wybierasz, ktora informacja bedzie pokazana, kiedy ty
listujesz biuletyny komenda DIR. Kazda opcja odpowiada jednemu
standardowemu formatowi.
Jesli ten standardowy format jest wyszczegolniony opcja jest ustawiona,
jesli standardowego formatu brak opcja nie jest ustawiona. Opcje moze
zostac ustawione/usuniete, jesli wpiszesz +/- z przodu tej opcji.

Przyklad:  A IL +Y   opcja Y (pokaz rok w dacie)

Patrz: HELP ALTER i HELP ARGUMENTY.

\\ALTER.IREAD

Ta komenda wybierasz, ktora informacja bedzie pokazana, kiedy ty
listujesz biuletyny komenda DIR. Kazda opcja odpowiada jednemu
standardowemu formatowi.
Jesli ten standardowy format jest wyszczegolniony opcja jest ustawiona,
jesli standardowego formatu brak opcja nie jest ustawiona. Opcje moze
zostac ustawione/usuniete, jesli wpiszesz +/- z przodu tej opcji.

Przyklad: A IR +Y
Patrz: HELP ALTER i HELP ARGUMENTY.

\\ALTER.L=ALTER.LINES

\\ALTER.LF

Syntaktyka: (LTER) LF [ -1 ... 5 ]
(wartosc domyslna: 0)

Numer pustych linii miedzy opisami. Z "A LF 1" dostajesz jedna pusta
linie wysuwu pomiedzy tekstami. Z "A LF 0" to niemasz zadnych pustych
lini. Jesli wartosc -1 jest zdefiniowana, dodatkowe puste linie tez sa
zlikwidowane.

Przyklad:   A LF 1   daje jedna pusta linie 
Patrz: HELP ALTER.

\\ALTER.LINES 

Syntaktyka:  A(LTER) L(INES) [<n>]
(wartosc domyslna: 0)

Ilosc linii tekstu, ktore BBS wysyla do czasu czekania na ENTER.
Wylaczenie L 0. 
Przyklad:   
        L 20   ustawia do 20 lini tekstu do momentu oczekiwania na ENTER 

Zobacz tez HELP ALTER i HELP STRONICOWANIE

\\ALTER.LINUXPW 

(tylko linux)
Syntaktyka:  A(LTER) LINUX(PW) <haslo> 

ALTER LINUXPW <haslo> definiuje haslo uzytkownika systemu linux.
Maksymalny dlugosc powinna byc 8 symbolami. Aby uaktywnic, te funkcje, 
ADDLINUXSYSTEMUSER musi zostac ustawiony na 1 w init.bcm!

Zobacz tez HELP ADDLINUXSYSTEMUSER

\\ALTER.LOGINPWTYPE

Syntaktyka:  ALTER SFPWTYPE   BAYCOM|MD2|MD5|INACTIVE
Sluzy do wyboru algorytmu dla hasla dostepu w trakcie retransmisji
wiadomosci. Kazdy z dostepnych algorytmow wykorzystuje ten sam tekst
wprowadzony za pomoca polecenia "alter pw".
Zobacz tez  HELP ALTER i HELP PASSWORD

\\ALTER.MYBBS

Odpowiada rozkazowi ALTER FORWARD. 
Patrz: HELP ALTER.

\\ALTER.NAME

Przyklad: A N Krzysztof
Patrz: HELP ALTER.

\\ALTER.NEWCALL

Syntaktyka:  A(LTER) NE(WCALL) [<znak>]

Jesli dostales nowy znak wywolawczy, mozesz w strukturze swojej skrzynki
z dawnym znakiem wywolawczym wpisac nowy znak do twoich starych ustawian
uzytkownika. Jesli poczta zostanie wyslana na stary znak to zostanie
automatycznie przekazana do twojego skrzynki z nowym znakiem.
Patrz: HELP ALTER

\\ALTER.NOPURGE

Wylaczenie rozkazu PURGE dla wlasnego katalogu.
Patrz: HELP ALTER i HELP PURGE CMDS.

\\ALTER.NOTIFICATION

Syntaktyka: Alter NOTification [<znak>]

Jesli masz znak klubu, i rejestrujesz sie pod tym znakiem bardzo czesto
to ma sens, abys byl informowany na Twoj znak za kazdym razem kiedy znak
klubu dostaje nowego maila. Dlatego mozesz uzyc ALTER NOTIFICATION do
tego.

\\ALTER.PACLEN 

(Sysop, czesc nieczynne)
Syntaktyka:  A(LTER) PAC(LEN) [<Bajt>]
(wartosc domyslna: 0)

Sysop moze strukture dla kazdego uzytkownika ustawic krotszy paclen.
Wartosc domyslna jest podana w PACLEN.
Syntaktyka dla sysopa:
                      SETUSER <znak> PACLEN 40..255  albo  0 

Komplet wartosci dla prawa uzytkowania moze zostac przedstawiona w
"ALTER <call> PACLEN", ale tylko jesli ty jestes sysopem i mozliwosc
wyboru PACLEN dziala.

Zobacz tez HELP PACLEN 

\\ALTER.PASSWORD=ALTER.PW

\\ALTER.PROMPT

Syntaktyka:  A(LTER) P(ROMPT) [<lancuch znakow>]

Ustawia podpowiedz programowa BBS. To jest mozliwe, by uzywac
makroinstrukcji w tej komendzie.

Przyklad:
  P (% b)->  pokazuje biezacy katalog 
  P          ustawia wartosc domyslna podpowiedziedzi 

Zobacz tez HELP MACROS, HELP DEFPROMPT i HELP ALTER

\\ALTER.PS 

Syntaktyka:  A(LTER) PS [<opcja>]

Opcje dla komendy PS.
Przyklad:   UR PS 
Patrz tez  HELP PS OPTIONS

\\ALTER.PW

Syntaktyka: A(LTER) PW [ <lancuch znakow> | OFF | DISABLE ]

Ta komenda mozesz ustawic twoje wlasne haslo, jesli umozliwi to sysop.
  a pw <lancuch znakow>  <lancuch znakow> bedzie dodany do istniejacego
                         hasla 
  a pw               Bez parametru twoje haslo jest usuniete 
  a pw off           Czyni nieaktywnym haslo, moze tylko reseted przez
                     sysopa 
  a pw disable       Sysop moze wylaczyc twoja funkcje hasla 

Rozmiar maksymalny jest 39 symbolami. Haslo nie moze zawierac znakow ";"
i ",". Jesli haslo jest ustawione i polaczysz sie z BBS, to wowczas nim
ukaze sie tekst powitalny ale cos podobnego do tego (zalezy od LOGINPWTYPE)
DB0AAB > 4 34 22 1 2 

Teraz musisz wyslac odpowiadajace znaki lancucha hasla. Jesli haslo jest
ustawione, twoj MYBBS moze tylko zostac zmieniony w BBS, ktory zostal
wyszczegolniony jako MYBBS wczesniej.

Z AT PW DISABLE sysop moze wylaczyc funkcje hasla.
Przyklad: A PW TO_JAM_JEST

Zobacz tez: HELP PASSWORD, HELP ALTER LOGINPW, HELP ALTER PWLINE
            i HELP ALTER READLOCK 

\\ALTER.PWLINE

Syntaktyka:  A(LTER) PWL(INE) [ 0 | 1 ]
(wartosc domyslna: 0)

W pogramach terminalowych musisz wyslac "pw" albo "sy" do BBS przed
reakcjami programu terminalowego podpowiedzi programowej haslo z BBS.
Jesli ustawiles PWLINE 1, ktore musisz wyslac linie BBS zanim to
przetestowuje haslo.
Wiec to jest mozliwe, by utworzyc haslo automatycznie w najwiecej ilosci
programow terminalowych.

Przyklad: A PWL 1
Zobacz  tez: HELP ALTER i HELP ALTER PW HELP ALTER PW 
 
\\ALTER.READLOCK

Syntaktyka:  A(LTER) REA(DLOCK) [ 0 | 1 | 2 ]
(wartosc domyslna: 0)

Jesli nie chcesz, zeby kazdy mogl przeczytac twoja poczte, mozesz zmienic
to uzywajac tej komendy.

  ALTER READLOCK 0, kazdy moze przeczytaz wszystkim twoje listy 
  ALTER READLOCK 1  w DIR/LIST/READ tylko posiadacz poczty i maili, 
                    ktore sa juz przeczytane ukaza sie 
  ALTER READLOCK 2  w DIR/LIST/READ posiadaczowi poczty ukaza sie 

Zobacz tez: HELP ALTER i HELP ALTER PW.

\\ALTER.REJECT

Syntaktyka: A(LTER) R(EJECT) [-R] <board lista> 

Tutaj mozesz wyszczegolnic katalogi, ktore nie beda w spise pokazane po
uzyciu komendy  CHECK lub NEWS DIR. Wiec mozesz wylaczyz nieporzadane
katalogi. Jesli wyszczegolnisz katalog glowny, NEWS DIR nie pokaze
podkatalogow takze. CHECK wylistuje podkatalogi, nawet, jesli katalog
glowny jest wyszczegolniony w wydruku. Opcja "-R" mozesz odwrocic
operacje wyboru. W ten sposob tylko okreslone wykazy beda pokazane.

Przyklady:  R DEBAT IBM             nie pokaze katalogow DEBAT i IBM 
            A R -R DXNEWS SOFTWARE  tylko wylistuje katalogi DXNEWS
	                            i SOFTWARE

NOWE: Tez nadawca albo obszary moga oprocz odrzucenia wydruku katalogow.
      Nadawca moze oznaczyc "<", area z "@".

Przyklad:    ALTER R DEBAT < DF3IAH @ WW 
             zamyka wszystkie linie, ze tylko jedno z trzech kryteriow 

Zobacz, jak HELP CHECK i pomoz DIR 
Patrz: HELP ALTER, HELP CHECK i HELP DIR.

\\ALTER.SFPWTYPE

Syntaktyka:  ALTER LOGINPWTYPE BAYCOM|MD2|MD5|INACTIVE
Sluzy do wyboru algorytmu dla hasla dostepu w trakcie interaktywnych 
lacznosci AX.25. Kazdy z dostepnych algorytmow wykorzystuje ten sam tekst
wprowadzony za pomoca polecenia "alter pw".
Patrz: HELP ALTER

\\ALTER.SPEECH

Syntaktyka: A(LTER) S(PEECH) [<jezyk>]

Przyklad: A S POL - polska wersja jezykowa, 
          A S XPL - full helpa sysopa
Patrz tez: HELP ALTER

\\ALTER.STATUS

Przyklad: A ST 0
Patrz: HELP ALTER.

\\ALTER.TTYPW

(tylko sysop)
Syntaktyka: A(lter) TTYPW [<haslo>]  SETUSER znak TTY haslo

Ustawia osobiste haslo uzytkownika dla TTY portu oraz TELNET i HTTP
Notka: maxymalna dlugosc jest 8 znakow

Przyklad: A TTYPW TO_TEZ_JA

Patrz tez: HELP ALTER.

\\ALTER.UDIR

Przyklad: A UD +Y
Patrz: H

Patrz tez: HELP ALTER i HELP ARGUMENTY.

\\ALTER.UFWD

Syntaktyka: Alter) UF(WD) <bbscall> <digicall> [<entry digicall>]

Przyklady:  A(LTER) UF(WD) PASSIVE
            A(LTER) UF(WD) OFF
	    A(LTER) UF(WD) OE1KDA-13 SR1BSZ OE1XLR
	    
Aktywuje sie user S&F.  
<boxcall> trzeba wpisac wlacznie z numerem SSID 

Patrz tez: HELP ALTER i HELP UFWD.

\\ALTER.ULIST

Przyklad: A UL +Y
Patrz: HELP ALTER i HELP ARGUMENTY.

\\ALTER.UREAD

Przyklad: A UR +Y
Patrz: HELP ALTER I HELP ARGUMENTY.

\\ALTER.UNSECURESMT

(Linux/Win32)
Syntaktyka: ALTER UN(SECURESMTP) [ 0 | 1 ]
(wartosc domyslna: 0)
Normalnie (wartosc "0") to jest mozliwe, by zrobic SMTP tylko po POP3,
e.g. poczcie moze byc tylko poslac przez TCP/IP, jesli inna poczta zostala
wybrana poprzednio. 
Z wartosc "1" to jest tez mozliwe, by zrobic SMTP bez poprzedzania POP3.
Ustawianie aby "1" moze byc dziurka bezpieczenstwa, specjalnie, jesli BBS
jest accessable przez internet.

Zobacz tez: HELP SMTP, HELP APOP

\\ALTER.ZIP

Syntaktyka: A(LTER) Z(IP) [<lancuch znakow>]

Mozesz zdefinowac zapisac - tekst lub kod pocztowy ta komenda

Przyklad: A Z PL-71454 

\\AMPRMAIL
Syntaktyka: amprmail

Komenda ta sprawdza czy na bramce jest jakas poczta dla uzytkownika.

\\AP=APPEND

\\APOP

Rozszerzenie nazwy pliku dla POP a authentification encoded, wiec POP
jest mozliwy jesli haslo dla zarejestrowania sie jest ustawione.
To rozszerzenie jest poparte w tej chwili przez telionalowy program "WPP".

\\APPEND

Syntaktyka: APPEND "tekst" zbior. Rozkaz sluzy do dodania linii tekstu do
podanego zbioru. Tekst nie moze zawierac srednika.
Przyklad: append "" test.txt dodaje pusta linie na koncu zbioru test.txt.

\\ARGUMENTY

Dla rozkazow READ, LIST, DIR, KOPF, HEADER i CHECK mozna podac
nastepujace dodatkowe argumenty (patrz takze pod: HELP ALTER, HELP
ARGUMENTY READ i HELP ARGUMENTY DIR):
  A   podanie znaku nadawcy.
  B   podanie dlugosci zbioru w bajtach.
  C   transmisja bez uwzglednienia podzialu na linie i strony.
  D   podanie daty.
  E   podanie znaku adresata lub nazwy katalogu.
  F   podanie znaku skrzynki nadawcy.
  G   ignorowanie przeczytanych wiadomosci (DIR/LIST/CHECK).
      pominiecie pola informujacego o przeczytaniu wiadomosci (READ).
  H   READ: R:-H wyswietlenie naglowka (wyklucza uzycie argumentu P).
      LIST/DIR/CHECK: pominiecie tytulu 'spis tresci'.
  I   podanie identyfikatora katalogu (BID).
  J   tytuly w stylu skrzynek 'DieBox' (DIR/LIST/CHECK).
  K   wyswietlanie nawiasu lub litery R po numerze wiadomosci.
  L   podanie czasu magazynowania.
  M   podanie adresu docelowego.
  N   zapytanie czy skasowac wiadomosc (READ).
      tylko wiadomosci, ktorych adresaci korzystaja z innych skrzynek.
      macierzystych (DIR/LIST/CHECK).
  O   podanie pierwszej czesci adresu docelowego.
  P   podanie znakow skrzynek w formie skroconej (tylko dla rozkazu READ)
  Q   wyswietlenie wiadomosci, ktore naplynely od czasu ostatniego.
      polaczenia (dotyczy tylko rozkazow LIST/DIR/CHECK).
  R   READ: podanie znakow stacji, ktore odczytaly wiadomosc.
      CHECK: podanie spisu wiadomosci skasowanych zdalnie.
  S   uwzglednienie podkatalogow w spisie (tylko dla rozkazow: 
      LIST/DIR/CHECK).
  T   podanie aktualnego czasu.
  U   podanie znaku nadawcy i jego skrzynki (tylko dla rozkazu READ).
  V   podanie listy skasowanych wiadomosci (patrz takze: rozkaz UNERASE).
  W   podanie tytulu wiadomosci.
  X   wprowadzenie dodatkowego odstepu przed tytulem (LIST/DIR/CHECK).
      pominiecie czesci dwojkowej zbioru (READ).
  Y   podanie roku w dacie.
  Z   podanie dlugosci zbioru w liniach.

\\ARGUMENTY.CHECK=ARGUMENTY.DIR

\\ARGUMENTY.DIR

Rozkazy LIST, DIR i CHECK moga zawierac nastepujace argumenty:
  A   podanie znaku nadawcy.
  B   podanie dlugosci zbioru w bajtach.
  C   transmisja bez uwzglednienia podzialu na linie i strony.
  D   podanie daty.
  E   podanie znaku adresata lub nazwy katalogu.
  F   podanie znaku skrzynki nadawcy.
  G   ignorowanie przeczytanych wiadomosci.
  H   pominiecie tytulu 'spis tresci'.
  I   podanie identyfikatora katalogu (BID).
  J   tytuly w stylu skrzynek 'DieBox'.
  K   wyswietlanie nawiasu lub litery R po numerze wiadomosci.
  L   podanie czasu magazynowania.
  M   podanie adresu docelowego.
  N   tylko wiadomosci, ktorych adresaci korzystaja z innych skrzynek
      macierzystych.
  O   podanie pierwszej czesci adresu docelowego.
  Q   wyswietlenie wiadomosci, ktore naplynely od czasu ostatniego
      polaczenia.
  R   spis wiadomosci skasowanych zdalnie (tylko CHECK).
  S   uwzglednienie podkatalogow w spisie.
  T   podanie aktualnego czasu.
  V   podanie listy skasowanych wiadomosci (patrz takze: HELP UNERASE).
  W   podanie tytulu wiadomosci.
  X   wprowadzenie dodatkowego odstepu przed tytulem.
  Y   podanie roku w dacie.
  Z   podanie dlugosci zbioru w liniach.

\\ARGUMENTY.FORWARD=OPCJE.FORWARD

\\ARGUMENTY.RETRANSM=OPCJE.FORWARD

\\ARGUMENTY.HEADER=ARGUMENTY.DIR

\\ARGUMENTY.KOPF=ARGUMENTY.DIR

\\ARGUMENTY.LIST=ARGUMENTY.DIR

\\ARGUMENTY.READ

Rozkazy READ moze zawierac nastepujace argumenty
(patrz takze HELP ALTER):
  A   podanie znaku nadawcy.
  B   podanie dlugosci zbioru w bajtach.
  C   transmisja bez uwzglednienia podzialu na linie i strony.
  D   podanie daty.
  E   podanie znaku adresata lub nazwy katalogu.
  F   podanie znaku skrzynki nadawcy.
  G   pominiecie pola informujacego o przeczytaniu wiadomosci
      (tylko dla wlasnych wiadomosci).
  H   wyswietlenie naglowka (wyklucza uzycie argumentu P).
  I   podanie identyfikatora katalogu (BID).
  K   wyswietlanie nawiasu lub litery R po numerze wiadomosci.
  L   podanie czasu magazynowania.
  M   podanie adresu docelowego.
  N   zapytanie czy kasowac wiadomosc.
  O   podanie pierwszej czesci adresu docelowego.
  P   podanie znakow skrzynek w formie skroconej.
  Q   (quiet) odczyt tylko czesci dwojkowej.
  R   podanie znakow stacji, ktore odczytaly wiadomosc.
  T   podanie aktualnego czasu.
  U   podanie znaku nadawcy i jego skrzynki (tylko dla rozkazu READ).
  W   podanie tytulu wiadomosci.
  X   pominiecie czesci dwojkowej zbioru.
  Y   podanie roku w dacie.
  Z   podanie dlugosci zbioru w liniach.

\\ARGUMENTY.RUNUTILS=ARGUMENTY.DODATKI

\\ARGUMENTY.DODATKI

Dostepne programy dodatkowe wymienione sa w zbiorze runutil.bcm. 
Format zbioru:
<rozkaz> "<program>"  <argumenty> ; <komentarz>

<rozkaz>  ...... rozkaz dla skrzynki
<program> ...... nazwa wywolywanego programu
<argumenty> .... argumenty decydujace o wywolaniu
<komentarz> .... wyjasnienie

Argumenty:
-s  tylko dla operatora.
-f  tylko w trakcie nawigacji w zbiorach.
-q  zbior rundat.bcm nie jest zakladany.
-c  w wywolaniu nie sa podawane zadne parametry.
-i  program nie przyjmuje danych od uzytkownika (Win32/Linux).
-p  tylko dla uzytkownikow z haslem dostepu AX25.
-d  (tylko Linux) zmienne ze sciezkami dostepu jak dla dpbox gesetzt 
    (dla 7get, 7mail, bsget)
-t  w parametrach moga sie znalezc znaki |,> i <  (tylko Linux)

Patrz tez: HELP DODATKI

\\ASKLOGIN

(tylko sysop, init.bcm)
Syntaktyka: ASKLOGIN [ 0 | 1 ]
(wartosc domyslna: 0)

Jesli ASKLOGIN jest ustawiony na "1" kazdy, ktory loguje się do BBS pierwszy
raz jest spytany o jego imieniu, qth, wpisz - tekst lub zip-kod i MYBBS.
Jesli BBS uzywa USERQUOTA ktory tez pyta, jesli on chce dostac aktywny czlon
tego BBS (dla zadnego downloadlimits, lepszego userstatus etc). 
Sysop dostaje poczte jesli reakcje uzytkownika z YES. 
Jesli ASKLOGIN jest ustawiony na "0" zadne zarejestrowanie sie pytajace
nie jest przedstawiony.

Zobacz tez: HELP USERQUOTA 

\\AUTOBIN=TRANSMISJE.DWOJKOWE

\\AUTOFWDTIME

(tylko sysop, init.bcm, _AUTOFWD opcje w config.h)
Syntaktyka: AUTOFWDT(IME) [<dzien>]
(wartosc domyslna: 0)
Ustawia numeracje dni, po ktorych stare marszruty sa zignorowane dla
wygenerowywania "afwd.bcm" z "afwdlist". 0 dni Disable autorouter.

Zobacz tez: H AFWDLIST, H AUTOROUTER i H AUTOPATH 

\\AUTOPATH 

(Opcja _AUTOFWD w config.h)
Syntaktyka: AUTOP(ATH) [-a] <boxcall> 
Przedstawia, czy autorouter zna sciezke dostepu do < boxcall>.
Kiedy uzywajac opcji "-a" allinformations <boxcall> jest przedstawiony.

Zobacz tez: HELP AUTOROUTER

\\AUTOROUTER 

(tylko sysop, _AUTOFWD opcja w config.h)
Autorouter tylko jest uzyty do S&F, jesli zaden route nie jest znaleziony
w "fwd.bcm".

"afwdlist"            tworzy plik "afwd.bcm" jesli "autofwdtime" nie jest 0. 
"autofwdtime 0"       Disable autorouter.
"autofwdtime n"       ustawia ilosc dni, po ktorych stare marszruty sa
                      zignorowane.
"autopath <boxcall>"  jest widocznym, jesli route do <boxcall> jest znaleziony
                      przez autorouter.

Plik "temp/afwd1.tmp" jest tylko dla informacji o marszrutach i nie uzywny.
Ten plik informacji ma format:
<S&F partner> <Adresat BBS-Callsign+Naglowek> 

Autorouter oblicza sciezke dostepu od numeru interweniowania innego 
BBS i opoznienie do poczty przychidzacej w BBS.

Zobacz tez: HELP AFWDLIST, HELP AUTOFWDTIME i HELP AUTOPATH 

\\AUTOSYSOP 

(tylko sysop, asysop.bcm)

Autosysop umozliwia automatyczna pozycje sysop na pewne porty tylko po
zalogowaniu sie.
Zapis hasla nie jest konieczny. Ustawiania mog zostac zredagowane rzedem 
"asysop.bcm" i powinien byc dobrym sekretem!

Format "asysop.bcm":
    polaczenie podrzedne lacza nadrzednego znaku 
Przykład:
    DJJ812 none none          (bezposrednio dajacy sie polaczyc)
    DJJ812 DBO812-2 DBO812-2  (jesli SSID portow zapisu jest -2)

Jesli w up- albo polaczenie podrzedne powinno nie zawrzec niczego, "none"
nie musi zostac zadeklarowany.
Polaczenie podrzedne jest znak wezla albo digipeater, ktory jest najblizej
BBS. Lacze nadrzedne jest znak wezla, ktory jest najdalszy od BBS,
takze ten wezel ponad ze uzytkownicy wchodzi do sieci pakietu.

Zauwaz:
Lacze nadrzedne moze latwo zostac sfalszowane, polaczenie podrzedne
teoreytcznie nigdy, odkad to jest zwykle wlasny jeden.

- lacze nadrzedne uzywajace logowania sie IP to jest adres IP, ktory moze
  tez zostac uzyty 
- jest lacze nadrzedne i polaczenie podrzedne oswiadczylo jako "none",
  to jest bezposrednio polaczony wezel albo digipeater miedzy tym. 
- Jesli digi nie posiada SSID 0 czy inaczej SSID, znak obowiazuje bez SSID 

\\AUTOTRCWIN

(tylko sysop, init.bcm, tylko DOS, _AUTOTRCWIN opcja w config.h)
Syntaktyka: AUTOT(RCWIN) [ 0 | 1 ]
(wartosc domyslna: 0)
Z "1" okno sladu powinno zostac wygenerowane automatycznie, inaczej off.

\\AX25JADRO=AX25K_IF

\\AX25K_IF

(Tylko dla systemu LINUX)
Syntaktyka: ax25k_if <zlacze> [d] off

Rozkaz sluzy do wlaczenia obslugi protokolu AX.25 przez jadro systemu
i do wprowadzenia danych konfiguracyjnych.
W polu nazwy zlacza podawany jest znak wywolawczy skrzynki.
Parametr "d" jest nieobowiazkowy i powoduje, ze zlacze to jest uzywane
domyslnie w polaczeniach nawiazywanych przez skrzynke.
W przeciwnym przypadku konieczne jest podawanie zlacza za pomoca rozkazu
ax25: <zlacze>. Patrz tez: HELP CONNECT.

\\BADNAMES

Syntaktyka: BADNAMES [haslo]. Rozkaz sluzy do przegladania ustawien filtru
wiadomosci znajdujacych sie w zbiorze "badnames.bcm". Zbior ten ma
nastepujacy format: symbol haslo
Dopuszczalnymi symbolami sa:
<  ..... nadawca
>  ..... adresat
@  ..... skrzynka docelowa
$  ..... identyfikator wiadomosci BID.
Hasla sa znakami stacji lub nazwami rubryk publicznych. Dopuszczalne jest
uzycie jokerow "*" i "?", ktorych zanaczenie jest identyczne jak w
systemie operacyjnym DOS.
Przyklad zawartosci zbioru badnames.bcm:
  -------------badnames.bcm
  < sp3vy
  > foto*
  > win*
  > dirty
  $ ??????RU0AAA
  @ SR8XXX
  -------------

\\B=BYE

\\BA=BATCH

\\BAKE=BEACON

\\BATCH

BATCH <nazwa pliku>
Sluzy do wywolania pliku wsadowego o podanej nazwie (<nazwa>.BAT),
a nastepnie zbioru <nazwa>.IMP. 
Rozkaz pozwala na sprawdzenie wywolan przez tabele CRONTAB.BCM.
W wersji dla systemu operacyjnego LINUX wykonywany jest jedynie zbior
<nazwa>.IMP. Patrz tez: HELP CRONTAB.

\\BAYCOM

BayCom jest nazwa zestawu komunikacyjnego packiet-radio skladajacego sie
z kontrolera USCC, modemu i odpowiedniego oprogramowania dla uzytkownikow
indywidualnych, wezlow i skrzynek sieci. Autorami sa krotkofalowcy DG3RBU
i DG8MBT.
Oprogramowanie wezla BayCom-Node pracuje na komputerach IBM-AT 
wyposazonych w kontroler komunikacyjny SCC. 
Maksymalna szybkosc transmisji wynosi 38400 bit/sek.
Wezel moze obslugiwac do 12 laczy radiowych. Program BCN jest programem
rezydentnym, dzieki czemu na tym samym komputerze moze pracowac program
skrzynki BayCom-Box (BCM).
Glownymi cechami charakterystycznymi wezla BCN sa:
- sposob obslugi (zestaw rozkazow) pokrewny z systemem THENET.
- protokol sieciowy FLEXNET.
- wyposazenie w rozszerzona funkcje przekaznikowa poziomu 2
  (ang. digipeating) z automatycznym wprowadzaniem znakow stacji
  posrednich (podobnie jak w oprogramowaniu FLEXNET). Funkcja ta pozwala
  na korzystanie z wezlow BCN na trasach polaczen TCPIP.
- trasy polaczen ze stacjami indywidualnymi rejestrowane sa na listach MH 
  Maksymalna dlugosc listy MH wynosi 1023 wpisy.
W 1995 r. oprogramowanie BCM zostalo zastapione przez PC-FLEXNET.
Oprogramowanie stacji indywidualnej sklada sie z rezydentnego programu
L-2 symulujacego kontroler TNC i programu terminalowego.

Zajrzyj na http://www.baycom.org

\\BCMNET

(Sorry, tylko w wersji jezykowej niemieckiej, poniewaz to tylko jest w
sieci CB w DL!)

\\BEACON

Syntaktyka: BEACON [tekst]
Powoduje natychmiastowe nadanie tekstu radiolatarni. Normalnie tekst
nadawany jest w porach zadanych w zbiorze CRONTAB.BCM.
Adres docelowy dla radiolatarni podany jest w zbiorze BEACON.BCM:
DOS: <wlasny znak> <cel> [ <przekaznik> ... ]
Linxu/Win32: <cel> [ <przekaznik> ... ]

\\BEGIN

BEGIN <nazwa zbioru>
Wyswietla poczatek podanego zbioru o dlugosci 2 kB.
Patrz takze: HELP TAIL.

\\BGET

Syntaktyka: BGET <nazwa zbioru>
Odpowiada rozkazowi RPRG, dotyczy serwera zbiorow. 
Patrz tez: HELP RPRG. 

\\BIDLIST

BIDLIST <kryterium>
Powoduje wyswietlenie czesci lub calosci spisu identyfikatorow wiadomosci 
(BID). 
Jako kryterium mozna uzyc nazwy zbioru albo fragmentu identyfikatora.
Przyklad: BID OE1KDA wywoluje spis identyfikatorow wiadomosci nadanych
przez OE1KDA. 

\\BIN=TRANSMISJE.DWOJKOWE

\\BOARD=KATALOG

\\BOXADRESS

Syntaktyka: BOXADRESS <adres hierarchiczny>
Sluzy do podania adresu hierarchicznego skrzynki.
Adres skrzynki zawiera jej znak bez identyfikatora wtornego, oznaczenia
rejonu i znormalizowanych oznaczen kraju i kontynentu, 
np: SR1BSZ.SZ.POL.EU.
Adres skrzynki o ograniczonym czasie pracy (np. prywatnej) sklada sie z 
wlasnego znaku, znaku regularnej skrzynki sieci i pozostalych oznaczen, 
np: SP1LOP@SR1BSZ.SZ.POL.EU
Ostatnio przyjelo sie podawanie na poczatku pola rejonu podwojnego
krzyzyka (#), ale to nie jest zasada i regula.
Patrz takze: HELP ADRESOWANIE.

\\BOXBIN=TRANSMISJE.DWOJKOWE

\\BOXHEADER

Syntaktyka: BOXHEADER <tekst>
Sluzy do wprowadzenia tekstu informacyjnego umieszczanego w naglowku
wiadomosci w linii oznaczonej symbolem "R:". Tekst dodawany jest tylko
do wiadomosci nadawanych, a nie do retransmitowanych.
Przyklad: BOXHEADER Szczecin JO73GL

\\BIULETYN

Oznacza wiadomosc adresowana do wszystkich (ogolnie dostepna).
Wiadomosci te moga byc kasowane jedynie przez nadawce i przez operatora
skrzynki.
Wiadomosci organizowane sa w postaci rubryk - katalogow tematycznych
(ang. info file).
Kazdej z rubryk przypisany jest okreslony czas przechowywania wiadomosci.
Oprocz biuletynow w sieci przekazywane sa wiadomosci prywatne
(ang. user file).

\\BYE

Rozkaz sluzacy do zakonczenia polaczenia ze skrzynka. W odroznieniu od
rozkazu QUIT nie powoduje on rejestracji czasu pozegnania ze skrzynka.

\\CALL

CALL <znak>  dostepne dla sysop
Podobny do rozkazu LOGIN, zamiast wywolania pelnej procedury zgloszenia
rejestruje jedynie zmiane znaku polaczonej stacji, np. w celu skorzysta-
nia z innych uprawnien lub konfiguracji. 
Patrz takze: HELP SETUSER.

\\CALLFORMAT

Syntaktyka: callformat 0|1|2
Ustala rodzaj znakow wywolawczych uznawanych za prawidlowe:
callformat 0  ... znaki krotkofalarskie
callformat 1  ... znaki CB (tylko niemieckie)
callformat 2  ... znaki krotkofalarskie i CB.
Parametr rozny od 0 powoduje dodanie do tekstu powitalnego ciagu "/CB". 

\\CD

CD <nazwa katalogu>
Pozwala na przejscie do pozadanego katalogu. Nazwa katalogu roboczego
moze byc podawana przez skrzynke w jej zgloszeniu.

\\CEDIT

!!Wystepuje tylko w wersji dla systemu DOS!!

Jest to synonim rozkazu EDIT CONVNAME.BCM. Po dokonaniu zmian w zbiorze 
nalezy podac rozkaz NEW dla zaktualizowania danych w pamieci.

\\CFGFLEX

Syntaktyka: CFGFLEX [<znak wezla>]
Nadaje zbior konfiguracyjny do podanego wezla FLEXNET. Zbior o nazwie
"cfgflex.bcm" musi znajdowac sie w katalogu skrzynki. W definicji lacza
z wezlem nalezy wpisac symbol "y".
Przyklad:
------------ cfgflex.bcm
w c
SR1BSZ - wezel w Szczecinie.
/ex
w l
Zainstalowana nowa wersja programu.
/ex

\\CHAT=TALK

\\CHECK

Syntaktyka: Check [<zakres> [[<] ["]<haslo>["]]

Wywoluje spis wiadomosci dla wszystkich. Lista uporzadkowana jest w
sposob odwrotnie chronologiczny (najnowsze wiadomosci znajduja sie na
poczatku). 
Podanie zakresu lub hasla pozwala na ograniczenie zawartosci listy do
interesujacego tematu lub okresu czasu. Haslo poszukiwania moze byc
podane w cudzyslowie lub bez. Hasla zawierajace znaki odstepu musza byc
podane w cudzyslowie. Duze i male litery w hasle nie sa rozrozniane.
Pola zawarte w nawiasach kwadratowych nie sa obowiazkowe. 
Zamiast rozkazu CHECK mozna uzyc rozkazow DIR NEWS lub DIR MESSAGES. 
Lista wywolywana przez rozkaz CHECK bez podania dodatkowych parametrow 
zawiera tylko wiadomosci, ktore naplynely od czasu ostatniej lacznosci 
ze skrzynka.
Przyklady:
  c                 wywoluje liste wiadomosci, ktore naplynely od czasu
                    ostatniej lacznosci.
  c 10 lub c 1-10   wywoluje liste ostatnich dziesieciu wiadomosci.
  c 1- oe1kda       wywoluje liste wiadomosci nadanych przez oe1kda.
  c ft470           wywoluje liste wiadomosci zawierajacych w tytule
                    haslo ft470. Duze i male litery w hasle nie sa
                    rozrozniane.

\\CLOG

Syntaktyka: CLOG ["]<haslo>["]
Wyswietla zakonczenie zbioru (ostatnie 2 kB) TRACE/CMDLOG.BCM.
Podanie hasla powoduje wyswietlenie wszystkich linii, w ktorych ono
wystepuje. Haslo zawierajace znaki odstepu musi byc podane w cudzyslowie.
Jako hasla moze sluzyc data lub znak wywolawczy. W zbiorze zawarty jest 
dziennik czynnosci operatora skrzynki.

\\CMD=ROZKAZY

\\COMMENT

Syntaktyka: comment [[<katalog>] <numer>] [<tytul>]
Sluzy do nadania odpowiedzi na odebrana wiadomosc. Podanie rozkazu bez 
parametrow oznacza powolanie sie na ostatnia odczytana wiadomosc. 
Potrzebne dane pobierane sa z naglowka wiadomosci. Dodatkowe parametry 
rozkazu pozwalaja na wybor innej zamiast ostatnio przeczytanej wiadomosci.

\\CONNECT

CONNECT [zlacze] <znak> <wezel>       (dostepny tylko z konsoli)
Pozwala operatorowi na polaczenie sie z dowolna stacja. Do przerwania
polaczenia sluzy rozkaz D. Po nawiazaniu polaczenia mozna podac automa-
tycznie haslo dostepu (do wezlow BayCom lub Flexnet) poslugujac sie
rozkazami SY lub PW. Hasla dostepu znajduja sie w zbiorze PASSWD.BCM.
Zbior ma nastepujacy format:
  SR1BSZ
  jhasgdhjgdhjgsjhgdjhgsjdhgsjhgdjhsghdjgashjgdhjsghjd
  sp6kdl
  sadjhsjhgdjhasgdhgshjdgahjsgdhjgshjdgahjsgdhgshjdghsj
  sr3box
  14711
  .......itd.

\\CP=TRANSFER

\\CONVERS=TALK

\\CONVERT

Syntaktyka: CONVER(T) [ a- | <lancuch znakow> ]

Jest plik "convert.bcm". Jesli dodasz  opcje "-a", caly plik jest 
pokazany. <lancuch znakow> jest posluzony jako wyrazenie skonczone. 
Ten plik jest uzywany dla automatycznego przesylania biuletynow. 
Tez maksymalny czas istnienia moze zostac zmieniony dla pojedynczej poczty. 
Jesli uzywasz <lancuch znakow> tylko lacza zawierające <lancuch znakow> 
jest przedstawiony. Jesli wprowadzasz tylko "convert" bez czegos innego,
ostatniego 2 kilobajtow "convert.bcm" jest pokazany.

Syntaktyka file:

; = komentarz
Lacze bez komentarza moze zawierac maksimum. 54 znakow. 
Z komentarzem to moze tylko byc 80 znakow wlaczajac. CR / LF!.

Standardowe formaty o specjalnym znaczeniu: < @ ~ $ % & 
 <call       nadawca 
 @call       BBS adresu albo region 
 ~word       przeszukiwanie slowa lub slowa w tytule 
 text        przeszukiwanie tekstu "tekst" w tytule 
Musisz uzyc %, $ i & bez zmiennych niezaleznych.
 $           odebrane z innego BBS przez S&F 
 %           wejscie uzytkownika 
 &           wejscie  uzytkownika bez hasla wejscie 

Uzyj tylko spacji zadnego tabulatora dla rozdzielania w laczach.

Rozkaz jest wazny, poniewaz tylko pierwszy zestawienie jest symboliczne. 
Badz swiadom porzadkujac lacza w "convert.bcm".

Kilka okolicznosci moze zostac zadeklarowane dla funktora logicznego AND
Aby siegnac OR uwarunkowac, uzywaja nowe lacze. 
Wszystkie okolicznosci musza zawsze zostac zestawiona. Duza litera alfabetu
albo male standardowe formaty wielkosci sa nieistotne.

Slowo jest otoczone z obszaru, dlatego ~NO nie znaczy " NO ", albo, jesli
to jest na poczatku tytulu: "NO " i, jesli to nie jest na koniec tytulu " NO".

Czas istnienia musi zostac zdefiniowany z trzema symbolami. 
Czas istnienia 000 ma na mysli ze maksymalny czas istnienia z pliku
"biuletyn.bcm" jest uzyty (nie Czas istnienia 0 dni!).
Czas istnienia 0 nie jest mozliwy z "convert.bcm", czas istnienia 1 jest
minimum wartosc i powinien wystarczyc, ale tylko, jesli minimalny czas
istnienia z "biuletyn.bcm" nie jest przewyzszony. 
Maksymalny czas istnienia z "biuletyn.bcm" moze zostaz przewyzszony
okolo "convert.bcm".

Jesli czas istnienia jest dodany do lacza konwertu, tez mode/scan tytul musi
byc dodany 'add'! Przyklad:

CQDX   DX          ; jest ok (przesylanie calej poczty z katalogu CQDX do DX)
CQDX   DX   005    ; jest niepoprawny (mode/scan brakuje tytulu)
CQDX   DX   005  $ ; jest ok (katalog przesyla CQDX do DX z czasem istnienia 
                   ; 5 dni, jesli przyjac zgodnosc via S&F)

Mozesz uzyć wyrazenia skonczonego z  <, @ i tekst, ale nie z  ~.

Symbol jokera "*" moze zostac uzyty, ale zapis z "*" kazdy dla 
zrodlo i katalog adresata nie jest mozliwy!

Zobacz tez: HELP REGULAR_EXPRESSIONS
 
\\CREATEBOARD

CREATEBOARD <parametr>
W zaleznosci od wartosci parametru pozwala uzytkownikom na zakladanie
katalogow:
0 - zakladanie wzbronione. Wiadomosci przeznaczone do nieistniejacych
    rubryk umieszczane sa w katalogu TMP.
1 - nowe katalogi zakladane sa w katalogu TMP, analogicznie jak dla
    retransmitowanych wiadomosci. Katalog TMP musi byc zalozony uprzednio
    przez operatora za pomoca rozkazu MKBOARD /TMP.
2 - Skrzynka nie przyjmuje wiadomosci przeznaczonych do nieistniejacych
    rubryk.
Pierwszenstwo maja uprawnienia zawarte w zbiorze CONVNAME.BCM.

\\CRONTAB

Oprogramowanie skrzynki wykonuje rozne zadania w ustalonym przez
operatora czasie. 
Rozkaz CRONTAB wywoluje spis procesow wraz z czasami ich wykonywania. 
Spis ten zawarty jest w zbiorze CRONTAB.BCM.

\\CRTSAVE

Syntaktyka: CRTSAVE <minuty>
Wystepuje tylko w wersji dla systemu DOS.
Powoduje wygaszenie ekranu po zadanym czasie oczekiwania na dane
z konsoli. 
Nacisniecie klawisza powoduje ponowne wyswietlenie tresci ekranu. Dla 
zaoszczedzenia energii zalecane jest calkowite wylaczenie monitora.

\\CZAS.MAGAZYNOWANIA=LIFETIME

\\DEFCHECK

Wybor standardowych wartosci argumentow dla rozkazu CHECK, np.
DEFCHECK  ABDEJLMWY.

\\DEFCMD

Syntaktyka: DEFCMD <rozkaz>
Definiuje rozkaz lub rozkazy wykonywane automatycznie po nawiazaniu
polaczenia ze skrzynka.
Wymienione rozkazy musza byc oddzielone przecinkami.

\\DEFGREP

Definiuje parametry dla rozkazu GREP.

\\DEFHELP

DEFHELP  0 | 1 | 2
Sluzy do wyboru sposobu zglaszania sie skrzynki.
Mniej doswiadczeni uzytkownicy powinni korzystac z poziomu 2.
Meldunek gotowosci zawiera wowczas spis najwazniejszych rozkazow. 
Poziom 1 powoduje skrocenie tekstu powitalnego (CTEXT).
Meldunek gotowosci na poziomie 0, przewidzianym dla doswiadczonych
uzytkownikow, nie zawiera spisu rozkazow.

\\DEFHTTPSURFACE 

(tylko sysop systemu, init.bcm, tylko Linux/Win32)
Syntaktyka: DEFHTTPSURFACE [ 0 | 1 | 2 ]
(wartosc domyslna: 1)

Z tym parametrem przedstawienie wartosci domyslnej wygladu uslugi http 
moze zostac wybrany:
  defhttps 0   prosty wyglad z ramkami i bez CSS - support 
  defhttps 1   mily wyglad z ramkami i z z CSS - support 
  defhttps 2   prosty wyglad bez ramek i bez CSS - support 

Plik "http/styl.css" moze zostac uzyty, by miec dalej ladny wyglad,
fonty, drugi plan - informacje wizyjne appearence kiedy CSS - jest
wybrany.
Jesli Webinterface jest uzyty powoduje spowolnienie szybosci transmisji
jesli uzyjesz polecenia wartosci 0, to zmiejszysz ruch danych za
kazdym polacznieniem.
CSS oznacza "Splywajacego kaskadami StyleSheets".

\\DEFIDIR

Definiuje standardowe wartosci parametrow rozkazu DIR dla rubryk
ogolnych (biuletynow).
Przyklad:
DEFIDIR   ADEKLMWYZ.

\\DEFILIST

Definiuje standardowe wartosci parametrow rozkazu LIST dla rubryk
ogolnych.
Ustawienie domyslne jest identyczne jak dla skrzynek DIEBOX.
Przyklad:
DEFILIST  ABDJQTWXY.

\\DEFIREAD

Definiuje standardowe wartosci parametrow rozkazu READ dla rubryk
ogolnych.
Przyklad:
DEFIREAD  ABDEILMPRTUWYZ.

\\DEFLF

Definiuje standardowa liczbe linii odstepu, np. DEFLF 0.

\\DEFLINES

Syntaktyka: DEFLINES <liczba>
Definiuje dlugosc nadawanej strony, wartosc 0 oznacza transmisje bez
przerw. 
Patrz takze: HELP STRONICOWANIE.

\\DEFPROMPT

Syntaktyka: DEFPROMPT <definicja>.
Definiuje zawartosc linii zgloszenia skrzynki. W tekscie moga byc zawarte
metasymbole.
Patrz takze: HELP ALTER i HELP METASYMBOLE.
Przyklad (zgloszenie w stylu Die-Box):
DEFPROMPT (%b) %c de %m>

\\DEFUDIR

Definiuje standardowe wartosci parametrow rozkazu DIR dla rubryk
prywatnych.
Przyklad:
DEFUDIR   ADKLMTWYZ

\\DEFULIST

Definiuje standardowe wartosci parametrow rozkazu LIST dla rubryk
prywatnych.
Przyklad:
DEFULIST  ABDJQTWXY

\\DEFUREAD

Definiuje standardowe wartosci parametrow rozkazu READ dla rubryk
prywatnych.
Przyklad:
DEFUREAD  ABDEILMPRTUWYZ

\\DELETE=ERASE

\\DIDADIT 

DODADIT protokol przekazywania 
Krotka forma: Dane Aa Danymi I Dane Sa Przenosne 

DIDADIT jest bardzo nowym protokolem przekazywania, ktory jest zamierzony,
by zastapic AUTOBIN i tryb przesylania YAPP. DIDADIT jest zdolny, by
strescic rozbite przesylania.
Spójrz na http://www.1409.org dla dalszych informacji o DIDADIT.
Nazwy zbioru musza byc mniejsz niz 50 znakowi, jesli one sa dluzsze
nimi chce zostac sciety wierzcholek, ale rozszerzenie nazwy pliku nazwy
zbioru bedzie trzymane.

Zobacz tez: HELP ALTER BINMODE, HELP FS DGET, HELP FS DPUT, HELP RDIDADIT 
i HELP WDIDADIT 

\\DIR

Syntaktyka: DIR [<rubryka>]
Powoduje wyswietlenie spisu tresci wybranej lub biezacej rubryki. Nazwa
rubryki podawana jest jako parametr rozkazu, w przypadku braku parametru
rozkaz dotyczy biezacej rubryki. 
Patrz takze: HELP KATALOG, HELP BIULETYN i HELP DIR <parametr>. 
Mozliwe jest podanie nastepujacych parametrow:
SZCZEGOLY, AFTER, BOARDS, MESSAGES, NEWS, OUTSTANDING, PATH, SENT i USERS

\\DIR.AFTER

DIR AFTER <data>                                               D A 25.07.
Odpowiada rozkazowi DIR MESSAGES, wyswietlana jest lista wiadomosci ktore
wplynely po podanej dacie. W przypadku opuszczenia miesiaca i roku
przyjmowany jest biezacy miesiac.

\\DIR.BOARDS

DIR BOARDS sluzy do wywolania spisu katalogow.                 D B

DIR BOARDS <nazwa>                                             D B PROGRAMY
Wywoluje spis katalogow w ktorych nazwach wystepuje podany tekst.
W spisie dodatkowo zawarty jest czas przechowywania wiadomosci i ich
liczba. Lista wszystkich katalogow w tym formacie moze byc wywolana za
pomoca rozkazu D B *.

\\DIR.SZCZEGOLY

DIR <katalog> <zakres>
Wywolanie czesciowego spisu tresci katalogu. 
Patrz takze: HELP ZAKRES.
Przyklady: D PROGRAMY       pelny spis tresci katalogu.
       D PROGRAMY -12       spis ostatnich 12 wiadomosci.
       D OE1KDA 1-          spis wszystkich wiadomosci dla OE1KDA.

DIR <nazwa katalogu> ["]<haslo>["]                           D IBM 7plus
Pozwala na ograniczenie spisu tresci do pozadanej tematyki. Otrzymany
spis tresci zalezny jest od parametrow podanych w rozkazie lub 
wprowadzonych za pomoca rozkazu ALTER. 
Patrz: HELP ALTER i HELP ARGUMENTY DIR.
Przyklady: D -G uwzglednia tylko wiadomosci nie przeczytane.
           D -L- nie wyswietla w spisie czasu skladowania wiadomosci.
Duze i male litery nie sa tu rozrozniane.
Patrz takze: HELP DIR <parametr>
Parametry: AFTER BOARDS MESSAGES NEWS PATH SENT USERS

\\DIR.MESSAGES

DIR MESSAGES                                                 D M
Wyswietla spis wiadomosci dla wszystkich. Spis ten moze byc
bardzo dlugi, dlatego zaleca sie ograniczenie go do interesujacych 
tematow.

\\DIR.NEWS

DIR NEWS                                                     D N

Odpowiada rozkazowi DIR MESSAGES, wyswietlany jest spis wiadomoci
ktore wplynely od czasu ostatniego uzycia tego rozkazu. Mozliwe jest
podanie tutaj takze nazwy katalogu. W odroznieniu od rozkazu CHECK 
wiadomosci sa grupowane wedlug katalogow.

\\DIR.OUTSTANDING

DIR OUTSTANDING <znak skrzynki> [<zakres>]

Sluzy do wywolania spisu poczty przeznaczonej do retransmisji do podanej
skrzynki. Wyswietlany jest najpierw spis wiadomosci prywatnych, a
nastepnie spis biuletynow. Kolejnosc wiadomosci w spisie jest zgodna z
kolejnoscia ich retransmisji. W celu ograniczenia dlugosci spisu mozna
podac zakres numerow wiadomosci.
Przyklad: 
D O SR1BSZ 1-20  - wywoluje spis pierwszych 20 wiadomosci przeznaczonych
                   dla SR1BSZ.

\\DIR.PATH

DIR PATHS                                                      D P
Podaje spis rubryk i polaczen wykorzystywanych do retransmisji.
D P <znak>   podaje trase polaczenia z podana stacja.

\\DIR.SENT

DIR SENT                                                       D S
Wyswietla liste wiadomosci zawierajacych w naglowku wlasny znak. 

\\DIR.USERS

DIR USERS                                                      D U
Wywoluje spis skrytek dla poszczegolnych uzytkownikow. Uwzglednione 
w nim sa takze wiadomosci skasowane tego samego dnia (jeszcze nie 
skasowane za pomoca rozkazu PURGE).

DIR USERS ALL [<znak>]                                         D U A SP9
Wywoluje liste wszystkich uzytkownikow skrzynki. W miare potrzeby
mozna podac czesc znaku wywolawczego.

DIR USERS LOCAL [<znak>]                                       D U L
Wywoluje liste uzytkownikow, ktorzy przynajmniej raz laczyli sie 
ze skrzynka.
Sposob poslugiwania sie jest podobny jak w przypadku rozkazu D U A.

DIR USERS MSG [<tekst>]                                         D U M
Wywoluje spis wiadomosci nadanych przez uzytkownikow (dla jej
ograniczenia mozna podac tekst, ktory musi wystapic w tytule).
Jest odpowiednikiem rozkazu DIR BOARDS dla biuletynow.

D -N USER MSG                                                   D -N U M
Wywoluje spis wiadomosci, ktorych odbiorcy nie korzystaja z wlasnej
skrzynki jako ze skrzynki macierzystej. Ulatwia to znalezienie 
niedostarczonych wiadomosci i zorientowanie sie czy operator skrzynki 
docelowej nie zaniedbuje swoich obowiazkow HI HI. 
Patrz takze: HELP ARGUMENTY.

\\DISABLE

DISABLE 0 | 1

Zamyka dostep do skrzynki. Dotyczy to zarowno uzytkownikow jak i 
retransmisji poczty. Dostep do skrzynki ma wowczas jedynie jej operator
po podaniu hasla.
Uzytkownicy polaczeni w tym czasie ze skrzynka musza byc rozlaczeni przez
operatora za pomoca rozkazu KILL.
Skrzynka po uruchomieniu znajduje sie w stanie pasywnym do czasu
wprowadzenia jej adresu hierarchicznego. Uzytkownicy probujacy polaczyc
sie ze skrzynka otrzymuja odpowiedni meldunek, na ktory mozna
odpowiedziec jedynie za pomoca rozkarzu PW lub SY i hasla operatora.
W przeciwnym przypadku nastepuje rozlaczenie.

\\DODATKI=RUNUTILS

\\DOSINPUT

DOSINPUT 0 | 1

Wystepuje tylko w wersji dla systemu DOS.
Okresla sposob wykonywania rozkazow systemu operacyjnego:
DOSINPUT 0  - Dane wyjsciowe programow moga byc wpisywane tylko do
              satandartowego kanalu wyjsciowego, programy nie moga
	      oczekiwac na dalsze informacje od uzytkownika.
DOSINPUT 1  - Dozwolone jest takze wprowadzanie informacji w czasie
              wykonywania programu.
W trybie tym nie wolno korzystac ze sterownika SHROOM, odkladajacego
zbedne dane z pamieci RAM na twardy dysk.

\\DZIENNIK=LOG

\\E=EDIT

\\EDYTOR=EDIT

\\EDIT

Syntaktyka: EDIT <zbior>

Wystepuje tylko w wersji dla systemu DOS.
Wywolanie edytora pozwalajacego na przetwarzanie zbiorow o dlugosci nie
przekraczajacej 60 kB. Wolny obszar pamieci RAM musi wynosic przynajmniej
60 kB. Edytor obslugiwany jest za pomoca klawiszy znacznika.
Dodatkowe rozkazy:
CTRL-Y     kasowanie linii,
CTRL-F     poszukiwanie zadanego ciagu znakow,
CTRL-L     dalsze poszukiwanie,
CTRL-N     skok do podanej linii,
ESC        zakonczenie pracy (z lub bez zapisu na dysku).

\\ENABLE

(tylko sysop) 
Syntaktyka: EN(ABLE) 

Sytan ZALACZONY jest napisany "init.bcm" jako "Disable 0". 
Aby umozliwic BBS do Disable stan. 

Zobacz też HELP DISABLE 

\\ELOG

Skladnia dla systemu Linux: ELOG ["]<haslo>["]
Skladnia dla systemow DOS/WIN32: ERLOG ["]<haslo>["]
Sluzy do wywolania zawartosci ostatnich 2 kB spisu skasowanych wiadomosci
Spis zawarty jest w zbiorze TRACE\ERASELOG.BCM. Podanie hasla powoduje
wyswietlenie wszystkich linii, w ktorych ono wystepuje. Hasla zawierajace
odstepy musza byc podane w cudzyslowie. W hasle mozna podac znaki stacji,
daty lub fragmenty tytulu.

\\ENABLE

Odwrotnosc rozkazu DISABLE, mozna tez uzyc DISABLE 0.

\\ERASE

Syntaktyka: ERASE <nazwa katalogu> <zakres>

Sluzy do skasowania podanych wiadomosci. Rozkaz bez parametrow kasuje
ostatnia przeczytana wiadomosc.
Przykladowo:E OE1KDA 3-4 albo E 1

Wiadomosci moga byc kasowane jedynie przez nadawce i adresata.
Rozkaz ERASE powoduje jedynie zaznaczenie w spisie wiadomosci jako
przeznaczonych do skasowania, dlatego tez mozliwe jest ich odzyskanie
za pomoca rozkazu UNERASE. Fizyczne skasowanie wiadomosci wymaga uzycia
rozkazu PURGE.
Numeracja wiadomosci w katalogu ulega zmianie dopiero po jej fizycznym
skasowaniu.

\\ERASELOG

Syntaktyka: ERASELOG <suma parametrow>
Ustala sposob zapisu w protokole TRACE/ERASELOG.BCM.
Zestaw parametrow:
Wartosc symbol znaczenie
  1      #L    rozkaz zdalnego kasowania podany ze skrzynki macierzstej
               nadawcy.
         #X    rozkaz zdalnego kasowania pochodzacy z innej skrzynki.
         #K    lokalny rozkaz zdalnego kasowania.
  2      #S    wiadomosci skasowane lokalnie przez operatora skrzynki.
  4      #E    wiadomosci skasowane lokalnie przez nadawce.
  8      #F    retransmitowane wiadomosci prywatne.
 16      #U    rejestracja rozkazow UNERASE.
 32      #T    biuletyny skasowane po wykonaniu rozkazu TRANSFER.

Przykladowe sumy parametrow:
ERASELOG 0    brak protokolowania,
ERASELOG 63   protokolowane wszystkie czynnosci.

Do przegladania zbioru sluzy rozkaz ELO ["]<haslo>["].
Argument +V w rozkazach DIR i LIST powoduje wymienienie w spisie takze
wiadomosci skasowanych. Zamiast numeru wiadomosci podawany jest wowczas
jeden z powyzszych symboli.

\\ERLOG=ELOG

\\EXIT=QUIT

\\EXPORT

Syntaktyka: EXPORT [-U] [-A] [-B] <zbior> <rozkaz>
Powoduje zapis meldunkow podanego rozkazu w zbiorze.
Przyklady: export ibm.txt r ibm 120-140
           export tekst d oe1kda 1-

Argument -A powoduje dopisanie meldunkow na koncu istniejacego zbioru 
            (ang. append). W przeciwnym przypadku zawartosc zbioru ulega
	    skasowaniu.
Argument -B wyswietlanie (tylko pod DOS).
Argument -U pozwala takze na zapis przez uzytkownikow (np. po zdalnym
            wywolaniu programow), w przeciwnym przypadku jest on dostepny
	    tylko dla operatora.
Przyklady: export -a test.txt ps
           export -u server.txt ps -abs
Podanie zamiast nazwy zbioru symbolu kanalu NUL powoduje ukrycie
meldunkow, np. export NUL OS gucio.exe

\\EXTRACT 

(tylko sysop, opcja DF3VI_EXTRACT w config.h)
Syntaktyka: EXTRACT [<boardname>] [<zakres>] 

Extract moze zostac uzyty, by uratowac BIN i 7PLUS pliki z maili. 
Pliki sa uratowane w informatorze TEMP katalogu BCM ("/bcm/temp").
BIN i 7PLUS pliki beda nadpisane bezlitosnie! Jesli nazwa zbioru BIN brakuje,
bedzie utworzony na nowo. Go/Stop znaczniki 7plus plikow nie jest uratowany.
Istniejace pliki nie sa zastapione ale ich rozszerzenie nazwy pliku bedzie
podliczalo. 

\\F>

Wlaczenie protokolu retransmisji.

\\FBB=ALTER.FBBCHECKMODE

\\FEDIT

Synonim rozkazu EDIT FWD.BCM.
Wystepuje tylko w wersji dla systemu DOS.
Po dokonaniu zmian nalezy wywolac rozkaz NEW w celu aktualizacji danych
w pamieci.

\\FILESERV

Usluga FILESERV sluzy do transmisji zbiorow analogicznie jak protokol FTP
Do zakoncznia sluza rozkazy BYE, EXIT lub QUIT. 
Patrz tez: HELP FS <rozkaz>.
Dopuszczalnymi rozkazami sa:
PATH, DIR, LS, CD, GET, PUT, BGET, BPUT, YGET, YPUT, QUIT.
Operator ma do dyspozycji dodatkowo: RM, MKDIR, RMDIR, CP i MV.

\\FILESERVER=FILESERV

\\FS
Usluga filesurf jesli zostanie zalaczona przez sysopa. Filesurf jest dobry
z mozliwoscia wymiany plikow.

- "FS" przelacznik z trybu BBS do trybu filesurf
- "Dir" list plikow, mozesz zmienic katalog z "CD"
- Do czytania plikow uzyc "GET", "", "BD.POLE Bget" i "YGET"
- Aby wyjsc z trybu filesurf i powrocic do trybu BBS uzyj "Q", "EXIT" 
  lub "quit"

Istnieje rowniez mozliwosc pozostawania w trybie BBS, ale potem trzeba
wpisac kazde polecenie z komendy prowadzacej FS.
Przyklad: FS DIR wymienia katalog glowny filesurf

Zobacz takze HELP <subcommand> FS:
PATH, DIR, LS, CD, GET, PUT, Bget, BPUT, BMAIL, BD.POLE, dput, YGET, YPUT, 
Jesli Sysop wylaczy to mozna rowniez korzystac z nastepujacych polecen: RM,
mkdir, RMDIR, CP, MV

Niektore BBSy korzystaja z dalszych runutils zainstalowanych do przesylania
plikow (np. 7plus-get bs-bin-transfer itp.).

\\FS.BGET

Syntaktyka: BGET <zbior>

Odczyt podanego zbioru w protokole Auto-Bin. 
Patrz tez: HELP TRANSMISJE.DWOJKOWE.

\\FS.BIN=FS.BGET

\\FS.BPUT

Syntaktyka: BPUT <plik>
Zapis zbioru w skrzynce za pomoca protokolu Auto-Bin.
Patrz tez: HELP TRANSMISJE.DWOJKOWE.

\\FS.BYE=FS.QUIT

\\FS.HELP=FILESERV

\\FS.CAT=FS.GET

\\FS.CD

Syntaktyka: CD <katalog>
Sluzy do zmiany katalogu roboczego wzglednie stacji dyskow ((DOS/Win32).

\\FS.CHDIR=FS.CD

\\FS.CMDMODE

Tryb interaktywny.

\\FS.COPY=FS.CP

\\FS.CP

Syntaktyka: CP <zbior1> <zbior2>
Sluzy do kopiowania zbiorow. W nazwach nie moga wystepowac jokery.
W nazwie zbioru docelowego nie mozna podac sciezki dostepu.

\\FS.DEL=FS.RM

\\FS.DGET

Syntaktyka: DGET <plik>
Czytaj <plik> z protokolem didadit transfer
Zobacz takze: HELP DIDADIT

\\FS.DIR

Syntaktyka: DIR <katalog> albo  ls <katalog>
Wywoluje spis tresci podanego katalogu w pelnej (DIR)
lub skroconej postaci (LS)

\\FS.EXIT=FS.QUIT

\\FS.GET

Syntaktyka: GET <plik>
Przeczytaj <plik> jako tekstowy plik.

\\FS.LS=FS.DIR

\\FS.MD=FS.MKDIR

\\FS.MOVE=FS.MV

Syntaktyka: RM <zbior>
Sluzy do skasowania podanego zbioru. W nazwie nie moga wystepowac jokery

\\FS.MKDIR

Syntaktyka: MKDIR <katalog>
Sluzy do zalozenia katalogu.

\\FS.MV

Syntaktyka: MV <zbior1> <zbior2>
Sluzy do zmiany nazwy zbioru.
W nazwach nie moga wystepowac jokery a w nowej nazwie - sciezki dostepu.

\\FS.PATH

Syntaktyka: PATH
Wywolanie spisu sciezek dostepu.

\\FS.QUIT

Syntaktyka: QUIT
Zakonczenie pracy i powrot na poziom skrzynki.

\\FS.RD=FS.RMDIR

\\FS.RMDIR

Syntaktyka: RMDIR <katalog>
Sluzy do skasowania katalogu.

\\FS.RM

\\FS.READ=FS.GET

\\FS.RPRG=FS.BGET

\\FS.RTEXT=FS.GET

Syntaktyka: GET <plik>
Wyswietlenie zawartosci zbioru tekstowego.

\\FS.RYAPP=FS.YGET

\\FS.TYPE=FS.GET

\\FS.WPRG=FS.BPUT

\\FS.WYAPP=FS.YPUT

\\FS.YGET

Syntaktyka: YGET <plik> 
Odczyt zbioru w protokole YAPP.
Patrz tez: HELP YAPP

\\FS.YPUT

Syntaktyka: YPUT <plik>
Zapis zbioru w protokole YAPP.
Patrz tez: HELP YAPP

\\FIND=PATH

\\FINGER=USERS

\\FLEXNET

Nazwe FLEXNET nosi oprogramowanie wezla pakiet-radio opracowane przez
DK7WJ. Pierwsza wersja pracowala na systemie mikroprocesorowym RMNC,
nastepnie wspolnie z DL8MBT zostala opracowana wersja dla komputerow PC
pracujaca pod systemem operacyjnym MS-DOS. Oprogramowanie FLEXNET
moze byc latwo dostosowane do potrzeb i wyposazenia komputera dzieki
znacznej liczbie dodatkowych sterownikow dla roznych urzadzen peryfe-
ryjnych. Programisci maja do dyspozycji dodatkowa biblioteke programow
i dokumentacje ulatwiajaca pisanie dalszych sterownikow.
Oprogramowanie FLEXNET wyposazenie jest w rozszerzona funkcje przekazni-
kowa poziomu 2 (ang. digipeating) z automatycznym wprowadzaniem znakow
stacji posrednich. Funkcja ta pozwala na korzystanie z wezlow FLEXNET
na trasach polaczen TCPIP. W wersji 32-bitowej oprogramowanie Flexnet
pozwala na korzystanie z przegladarek nternetowych w lacznosciach AX.25
Patrz takze: HELP FLEXNET ROZKAZY.

\\FLEXNET.ACTUAL=FLEXNET.AKTUELL

\\FLEXNET.AKTUALNOSCI=FLEXNET.AKTUELL

\\FLEXNET.AKTUELL

Rozkaz sluzy do odczytu aktualnosci wprowadzonych przez operatora wezla.
Sa one zawarte w zbiorze A.FPR.

\\FLEXNET.BCM

Polaczenie skrzynki BCM z wezlem FLEXNET.
a) oba programy pracuja na tym samym komputerze
   Skrzynka polaczona jest "wewnetrznie"  z kanalem 15 wezla FLEXNET.
   Konfiguracja:  MODE 15 y       - automatycznie dostep dla operatora,
               P T 0 15        - TXDelay bez znaczenia,
               P S 15 15       - SSID (np. 15),
               L 15 <znak>-8 @ - wpisanie skrzynki jako partnera na laczu
   Znak skrzynki musi byc tez wprowadzony za pomoca rozkazu 
   MYCALL <znak>-8.
b) programy skrzynki i wezla pracuja na oddzielnych komputerach:
   Za pomoca rozkazu FSET (MODE, PARMS) nalezy zadeklarowac uzywany 
   sterownik (np. Ethernet).
Przyklad:    FSET MODE 0 19200c   - CRC-KISS z szybkoscia 19200 bit/sek,
             FSET TXD  0 1        - TxDelay 10ms analogicznie po stronie
                                    wezla:
             MODE 7 19200c        - CRC-KISS z szybkoscia 19200 bit/sek,
             P T 1 0              - TxDelay 10ms
             P S 5 5              - SSID (np. 5)
             L 5 SR1BSZ-8 @       - lacze do skrzynki.
Lacze deklarowane jest za pomoca rozkazu L <kanal> <znak>-8 @.
W wykazie tras retransmisji dla skrzynki nalezy zawsze wymienic znak
lokalnego wezla FLEXNET, np. ...AA <cel> <wezel lokalny>.

\\FLEXNET.BEACONS

Rozkaz wywoluje konfiguracje radiolatarni wezla zawarta w zbiorze B.FPR. 
Konfiguracja zawiera teksty radiolatarni i ich przypisanie do poszczegol-
nych kanalow radiowych. Zbior B.FPR ma nastepujacy format:
<min> <kanal> <cel [via <znak> [<znak>...]]> : <tekst> #
Znaczenie pol:
"#"      oddziela poszczegolne teksty.
<min>    odstep czasu miedzy transmisjami tekstu. 
         (zakres: 0...255 [min]; 0 - transmisja wylaczona).
<kanal>  numer kanalu.
<cel>    pole zawiera znak lub oznaczenie adresata radiolatarni, mozna tu
         uzyc dowolnych oznaczen, jak: "BEACON", "RMNC","FLXNET","TEST"
	 itp.
via      pozwala na wprowadzenie do 8 znakow stacji przekaznikowych
         retransmitujacych tekst radiolatarni.
Przyklad: #10 0 FLXNET:Opole, kanal 144.650
          #30 1 FLXNET:Opole, lacze do SR9ZDN
          #5  2 FLXNET:Opole, wejscie 9600, kanal R61.
W zbiorze zawarte sa trzy teksty zakonczone znakami CR.
Pole nadawcy pakietu zawiera znak wezla.
Przed zdefiniowaniem tekstow nadawany jest w kanale 0 tekst standardowy
w odstepie 3-minutowym.

\\FLEXNET.KONFERENCJE=FLEXNET.CONVERS

\\FLEXNET.CONVERS

Convers
Powoduje przejscie do trybu konferencyjnego. Uzytkownik ma do dyspozycji
nastepujace rozkazy:
/w             Wywolanie spisu stacji polaczonych z wezlem.
/w n           Wywolanie spisu uzytkownikow danego kanalu konferencyjnego
/c             Odpytanie numeru uzywanego kanalu.
/c n           Przejscie do podanego kanalu konferencyjnego.
/s znak tekst  Nadanie wiadomosci do podanego uzytkownika.
/t             Zakonczenie dialogu.
/t znak        Rozpoczecie dialogu z podana stacja.
/q             Zakonczenie konferencji.

\\FLEXNET.INFO

Tekst informacyjny znajdujacy sie w zbiorze I.FPR zawiera
najczesciej opis wezla i jego wyposazenia.

\\FLEXNET.IO

Rozkaz nie wystepuje w wersji PC/Flex.

\\FLEXNET.L2STATES=FLEXNET.STANY_L2

\\FLEXNET.STANY_L2

W tabeli polaczen wyswietlane sa stany protokolu AX.25:
 1  Rozlaczenie.
 2  Nawiazywanie polaczenia.
 3  Odrzucenie pakietu.
 4  Zadanie rozlaczenia.
 5  Wymiana informacji.
 6  Nadany pakiet REJ.
 7  Oczekiwanie na pokwitowanie.
 8  Stan zajetosci.
 9  Stan zajetosci korespondenta.
10  Stan zajetosci obu stron.
11  Oczekiwanie na pokwitowanie i stan zajetosci.
12  Oczekiwanie na pokwitowanie i stan zajetosci korespondenta.
13  Oczekiwanie na pokwitowanie i stan zajetosci obu stron.
14  Nadany pakiet REJ i stan zajetosci.
15  Nadany pakiet REJ i stan zajetosci korespondenta.
16  Nadany pakiet REJ i stan zajetosci obu stron.

\\FLEXNET.LINK

LINK <kanal | znak | -> <znak> [#|$|-|@|>|)|!]
Przyporzadkowanie sasiednich wezlow i skrzynek do kanalow laczy.
Znaki stacji moga zawierac identyfikatory SSID.
Ostatnie pole moze zawierac nastepujace parametry:
a) lacza bez pomiaru czasu propagacji:
  '$' - brak pomiaru czasu propagacji pakietu.
  '#' - j.w.; lacze ukryte przed uzytkownikami.
b) lacza z pomiarem czasu propagacji pakietu:
  ' ' - pole puste; normalny pomiar i wymiana informacji o laczach sieci.
  '-' - informacja o laczu do stacji partnerskiej nie jest przekazywana 
  dalej, informacje o celach przez nia osiagalnych - tak.
  '>' - ukrycie informacji o wszystkich celach osiagalnych w tym kanale.
  '!' - przekazywana wylacznie informacja o stacji partnerskiej bez
        podania dalszych celow.
  '@' - probny pomiar czasu propagacji bez uzycia protokolu sieciowego
  Flexnet, np. w kanalach prowadzacych do wezlow TheNet.
  ')' - sasiadem jest siec lokalna, lacze ukryte (jest to kombinacja 
  z '>' i '#').
Przyklady: 
  1) 'link 2 sr9zdn'      : kanal 2 jest laczem z sr9zdn.
  2) 'link 6 sp9yai-7 #'  : ukryte lacze z SP9YAI-7 w kanale 6
  3) 'link 15 sp6kbl-9 )' : ukryte lacze do retransmisji ze skrzynka
                            SP6KBL w kanale 15.
  4) 'link sr6bbs sp6djg' : lacze do  SP6DJG przez znane polaczenie
                            z SR6BBS.
W odroznieniu od BCN dopuszczalna jest deklaracja wiekszej liczby
polaczen prowadzacych do tego samego celu za posrednictwem roznych
kanalow.
Skasowanie wpisu w tabeli polaczen nastepuje za pomoca rozkazu
LINK- <znak>. W polaczeniach z sasiednimi wezlami FLEXNET nie podaje sie
zasadniczo identyfikatora SSID - dozwolony jest pelny zakres od 0 do 15.

\\FLEXNET.LOCAL

Do stacji laczacych sie bezposrednio (lokalnie) z wezlem nadawany jest 
dodatkowy tekst powitalny zawarty w zbiorze L.FPR. Moze on byc tez 
wywolany za pomoca rozkazu LOCAL.

\\FLEXNET.MHEARD

MHeard [znak] [kanal | n]
Wywolywana jest lista bezposrednio odbieranych stacji. Jest ona
automatycznie rejestrowana w zbiorze MH.FPR w odstepach 10 minutowych.
Zbior ten zakladany jest automatycznie przez oprogramowanie wezla.

\\FLEXNET.MODE

MODE <kanal> <parametry>
Sluzy do konfiguracji poszczegolnach kanalow wezla. 
Do dyspozycji stoja nastepujace parametry:
<liczba> - szybkosc transmisji, dozwolone sa wielokrotnosci 300 bit/s;
      zera na koncu liczby moga byc opuszczone.
'c' - w laczu KISS uzywana suma kontrolna; dla plytek SCC i modemow
      BayCom - programowe rozpoznawanie sygnalu (DCD).
'd' - pelny dupleks, standadowo - poldupleks.
'm' - protokol DAMA
'r' - odbiorczy sygnal zegarowy pochodzi z modemu (np. z modemu G3RUH).
's' - kanaly zsynchronizowane (np. dla dwoch kanalow na tej samej 
      czestotliwosci).
't' - nadawczy sygnal zegarowy pochodzi z modemu (np. z modemu G3RUH).
'u' - kanal uzytkowy (pomiar TXDelay, protokol DAMA).
'y' - automatycznie uprawnienia operatora (bez podawania hasla).
'z' - kodowanie NRZ (np. dla modemu DF9IC); standardowo kodowanie NRZI.
'-' - kanal wylaczony.
'.' - parametr jalowy, gdy zmiany zbedne.
Przyklady: 1) 'mode 2 12cmu' : 1200-bit/sek/s-kanal uzytkowy, modem
                               BayCom, protokol DAMA-Modus w kanale 2.
     2) 'mode 3 96dtrz': 9600-bit/sek-lacze pelnodupleksowe, modem 
                         DF9IC-FSK w kanale 3.
     3) 'mode 4 -'     : kanal 4 wylaczony (mozliwe tylko dla 
                         niektorych sterownikow!).
Zestaw parametrow i dokladne znaczenie parametrow zalezne sa od
sterownika.

\\FLEXNET.PARMS

PARMS <parametr> <..> <..>

Sluzy do zmiany wartosci TXDelay i do przypisania identyfikatora wtornego 
SSID dla poszczegolnach kanalow.
a) zmiana wartosci TXDelay w podanym kanale:
  P T <txd> <kanal>
          zakres wartosci wynosi 1 - 255 i jest wielokrotnoscia 10 msek.
          Przyklad: P T 12 3 - czas TXDelay 120 msek w kanale 3.
b)  zmiana identyfikatora SSID w podanym kanale:
  P S <ssid> <kanal | 16>
    zakres wartosci pokrywa sie z ustalonym za pomoca rozkazu MYCALL.
          Przyklad: P S 15 15
    Zadany identyfikator jest kasowany przez przypinanie go do jednego z 
    nieistniejacych kanalow, np. kanalu 16. Kanaly bez przypisanego 
    identyfikatora SSID moga byc wykorzystawane jedynie w laczach sieci. 
    Kanaly uzytkowe musza miec przypisany identyfikator.

\\FLEXNET.RESET

RESET
Rozkaz nie wystepuje w wersji PC/Flex. Zamiast niego mozna uzyc rozkazu
SHUTDOWN -R dla skrzynki BCM lub jezeli wezel PC/Flex pracuje na osobnym
komputerze, do wylaczenia go nalezy posluzyc sie SERV.

\\FLEXNET.RESTART=FLEXNET.RESET

\\FLEXNET.ROZKAZY

Uzytkownik wezla FLEXNET ma do dyspozycji nastepujace rozkazy:
Aktuell/Actual             Odczyt aktualnosci.
Beacons                    Konfiguracja radiolatarni.
Convers                    Przejscie do trybu konferencyjnego.
Connect <znak> [wezel | kanal] Nawiazanie polaczenia z podana stacja.
Destinations [*] [znak]    Wywolanie spisu stacji docelowych; trasa do 
                           podanej stacji.
Destinations <wezel> "*"   Wyswietlenie czasow propagacji pakietow na 
                           roznych trasach.
Destinations <wezel> ">"   Dokladny wykaz czasow propagacji
Find <znak>                Poszukiwanie danej stacji.
Help                       Wywolanie tekstu pomocy.
Info                       Wywolanie tekstu informacyjnego.
Links [*]                  Wywolanie informacji o trasach polaczen.
LOcal                      Wywolanie lokalnego tekstu powitalnego.
Mail [?]                   Polaczenie z najblizsza skrzynka elektroniczna
MHeard [znak] [kanal | n]  Wywolanie spisu odbieranych stacji.
MYcall                     Wywolanie spisu znakow wywolawczych wezla.
Port [*] [kanal]           Wykaz parametrow warstwy 1/2 (ew. dla danego
                           kanalu).
Quit                       Przerwanie polaczenia
Setsearch                  Wykaz tras poszukiwania (uzywanych przez
                           rozkaz FIND).
STat                       Wywolanie danych statystycznych.
Talk <znak> [tekst]        Nadanie wiadomosci do podanej stacji.
Users [*] [kanal|"="]      Wywolanie spisu uzytkownikow, ew. tylko dla 
                           podanego kanalu.
         "=" uwzglednia tylko uzytkownikow polaczonych z wezlem.

Patrz tez HELP FLEXNET <parametr>:
AKTUELL BEACONS CONVERS INFO LOCAL MHEARD SETSEARCH 
RADIOLATARNIA KONFERENCJE STANY_L2

\\FLEXNET.SETSEARCH

Rozkaz sluzy do wprowadzania tras poszukiwania stacji uzywanych przez 
rozkaz FIND. Spis ten, znajdujacy sie w zbiorze S.FPR ma nastepujacy
format:
   <znak1>
   <znak1> [<znak2> [<znak3> [<znak4> [<znak5>]]]]
Przyklad:
  SR6BBS
  SR6BBS-2
  SP6FIG via SR6BBS
  SR6DJG via SR6BBS
Pierwsza linia zawiera znak wezla (SR6BBS), druga linia okresla kanal 
poszukiwania (SR6BBS-2). Dalsze linie zawieraja znaki poszukiwanych 
stacji indywidulanych i wezlowych.
Pierwsza linia moze zawierac kreske laczaca (-) zamiast znaku wezla, 
druga - kreske i identyfikator SSID. 
Przyklad:
   -
   -2
   SP6FIG -
   SR6DJG -

\\FLEXNET.SYSOP

Operator wezal FLEXNET na do dyspozycji nastepujace rozkazy:
CAL <kanal> [min | 0] - nadawanie sygnalu kalibracyjnego przez czas
                 zadany w minutach i w podanym kanale. Czas 0 minut
		 oznacza zakonczenie nadawania. Wartoscia domyslna jest
		 1 minuta.
KILL <nr. qso> - przerwanie polaczenia i skasowanie wpisu w tabeli 
                 uzytkownikow.
Links <kanal | lacze> <znak> [#|$|-|@|>|!|)]
                 uzupelnienie tabeli laczy.
Links - <znak> - skasowanie wpisu w tabeli.
Mail <znak>    - zadeklarowanie podanej stacji jako skrzynki.
MYcall <znak> [ssid1 ssid2]<znak> podanie znaku dla funkcji przekaznikowej
                 wezla, ew. z dodatkiem zakresu identyfikatorow.
MOde <kanal> <parametry> - konfiguracja kanalow.
Parms S <ssid> <kanal|16> - zadeklarowanie SSID w danym kanale.
Parms T <txdelay> <kanal> - zmiana czasu TXDelay w danym kanale.
SYsop          - zgloszenie sie na poziomie uprawnien operatora.
TRace <kanal> [znak] [argumenty] - podglad (monitorowanie) danego kanalu,
                 mozliwe ograniczenie sie do podgladu podanej stacji.
TRace          - zakonczenie monitorowania.
WRITE <A|B|C|H|I|L|S> - WRITE, zapis zbiorow tekstowych (zakonczenie: 
                 /ex albo ^Z). Zestaw rozkazow dla uzytkownika wywolywany
		 jest przez HELP FLEXNET ROZKAZY.
Patrz takze HELP FLEXNET <parametr>:
AKTUELL, BCM, BEACONS, ROZKAZY, KONFERENCJA, CONVERS, INFO, IO,
L2STATES, LINK, LOCAL, MHEARD, MODE, PARMS, RESET, RESTART, 
SETSEARCH, SYSOP, TRACE, WRITE, RADIOLATARNIA, AKTUALNOSCI.
Sposob polaczenia wezla FLEXNET ze skrzynka BCM opisany jest pod:
HELP FLEXNET BCM.

\\FLEXNET.TRACE

Skladania: TRace <kanal> [znak] [argumenty]

Umozliwia podglad (monitorowanie) podanego kanalu, z ew. ograniczeniem
sie do interesujacej stacji. W wezlach pracujacych na wspolnym komputerze
ze skrzynka BCM musi byc wylaczony podglad skrzynki. Dane z naglowkami
skomprymowanymi nie sa wyswietlane.
Argumenty:
 '#' - Pakiety RR/RNR/REJ nie wyswietlane.
 '$' - Pole informacyjne pakietu nie wyswietlane.
 '>' - Wyswietlane tylko pakiety nadawane.
 '<' - Wyswietlane tylko pakiety odbierane.
Rozkaz TRACE bez parametrow powoduje zakonczenie podgladu.

\\FLEXNET.WRITE

Syntaktyka: WRITE <flex-flex>

Zapis zbioru tekstowego na dysku. Zakonczeniem zbioru jest ^Z albo /EX.
<flex-flex>=A, B, C, H, I, L, S
Zbiory otzzymuja odpowiednio nazwy A.FPR itd. Praktyczniejsza metoda jest
zapis zbiorow w skrzynce BCM korzystajac z rozkazow Rt albo RPRG.
a) zbiory tekstowe A.FPR, C.FPR, H.FPR, I.FPR, L.FPR.
   Zawartosc i format - dowolne. Zalecane jest umieszczenie na poczatku
   zbioru znaku CR dla zwiekszenia czytelnosci, wyjatkiem moze byc tekst
   powitalny zawarty w zbiorze C.FPR.
b) zbiory B.FPR i S.FPR maja specjalny format, 

patrz HELP FLEXNET BEACONS i HELP FLEXNET SETSEARCH.

\\FLOATS=ROZDZIELNIK

\\FLXNET=FLEXNET

\\FOLLOWUP=COMMENT

\\FORWARD

Syntaktyka: FORWARD [-F] [-H] <nazwa katalogu> <zakres> [ @ ] <adresat>

Powoduje retransmisje wiadomosci do podanej skrzynki. Rozkaz ten moze byc
uzyty jedynie przez nadawce lub adresata danej wiadomosci. Bez podania 
zakresu rozkaz odnosi sie do ostatnio przeczytanej wiadomosci.
Przyklady: f info 5 sp
           f oe1kda 3-4 @ sr6bbs
Po retransmisji wiadomosci prywatne sa kasowane, wiadomosci ogolne sa  
kopiowane do skrzynki docelowej. W miare potrzeby znak skrzynki
docelowej musi byc uzupelniony pelnym adresem hierarchicznym (dotyczy
to zwlaszcza odleglych skrzynek zagranicznych). 
Wiadomosc retransmitowana jest bez zmian w naglowku (bez zmiany adresu 
docelowego). Zmiana adresu docelowego nastepuje po podaniu w rozkazie
parametru -F.
Normalnie retransmitowane sa wiadomosci nie zawierajace w naglowku
ustawionego bitu "h". Retransmisja wiadomosci z ustawionym bitem
wymuszana jest przez podanie parametru "-h". 
Patrz takze: HELP USERSF i HELP ADRESOWANIE.

\\FORWARD.ARGUMENTY=OPCJE.FORWARD

\\FWDSSID

Syntaktyka: FWDSSID <n>
Podaje identyfikator wtorny dla retransmisji inicjowanych przez skrzynke.
W rzeczywistosci uzywany jest naprzemian podany identyfikator i identyfi-
kator o jeden wyzszy dla unikniecia niejasnosci. Podany tutaj identyfika-
tor powinien byc wyzszy od normalnie uzywanego przez skrzynke lub co
najmniej o dwa nizszy.
Przyklady: 
  mycall SR1BSZ-8 => dozwolony fwdssid  0,1,2,3,4,5,6,9,10,11,12,13,14.
  mycall sr3box-0 => dozwolony fwdssid 1..14.
Wynika stad np. fwdssid 9.

\\FWDTIMEOUT

Syntaktyka: FWDTIMEOUT <minuty>

Ustala czas oczekiwania na odbior wiadomosci. 
Po jego uplywie polaczenie zostaje przerwane. Czas ten zalezny jest od
jakosci i obciazenia lacza. Przerwanie polaczenia jest rejestrowane w
dzienniku skrzynki i opatrzone symbolem #L. Zbyt czeste przerywanie
retransmisji wymaga podwyzszenia granicy albo poprawy jakosci lacza.
Przyklad: fwdtimeout 60
Patrz tez: HELP FORWARD ARGUMENTY, HELP FWDSSID i HELP FWDTRACE.

\\FWDTRACE

Syntaktyka: FWDTRACE <parametr>

Ustala sposob protokolowania retransmisji w zbiorze TRACE/T_<znak>.BCM.
Rejestrowane sa: 
czas i data, numer procesu, kierunek (S- nadawanie, R - odbior) i tytul.
Do przeszukiwania protokolow sluzy rozkaz TGREP <haslo> <znak>. 
Rozkaz TGREP <znak> wyswietla zakonczenie odpowiedniego zbioru 
(ostatnie 2 kB).
Znaczenie parametrow:
fwdtrace 0  : protokolowanie wylaczone.
fwdtrace 1  : protokolowanie wlaczone.
fwdtrace 2  : protokolowanie retransmisji do skrzynek opatrzonych
              w zbiorze FWD.BCM symbolem -T.
Patrz tez: HELP FORWARD ARGUMENTY, HELP FWDTIMEOUT i HELP FWDSSID.

\\FTS 

Syntaktyka: FTS [zakres] <lancuch znakow> ["<board>"]

Z tym komenda przeszukiwanie fulltext przez calej poczty albo cala poczta
jednego katalogu moze zostaz zaczeta.
Duze mailboxy i powolniejszy sprzet komputerowy wiecej razu, ktory to bierze!

Przyklady:  FTS tnc4            szuka w calej poczcie lancuch znaków "tnc4"
            FTS 1-1000 kenwood  szuka w 1000 najnowszych mailach lancuch
                                znakow "kenwood"
            FTS tm733 "technik" szuka w calej poczcie w katalogu TECHNIK 
                                lancuch znakow "tm733"

Zauwaz: Nie ma zadnej roznicy w skutku kiedy piszac <lancuch znakow> z
duzej litery alfabetu albo mala standardowe formaty papieru o wielkości!
Jesli katalog jest zdefiniowany, zakres jest zignorowany.

\\GREP

Syntaktyka: GREP ["]<haslo>["] <nazwa zbioru>

Sluzy do poszukiwania w zbiorze zadanego hasla. Wyswietlane sa pelne
linie, w ktorych ono wystepuje. Hasla zawierajace znaki odstepu musza
byc podane w cudzyslowie.
Przyklad: grep "start" trace/syslog.bcm

\\GUESTCALL

Syntaktyka: guestcall <znak>|off

Ustala znak dla gosci skrzynki. Korzystajacy z niego maja ograniczone
uprawnienia np nie moga nadawac wiadomosci.

\\HADRSTORE

Syntaktyka: HADRSTORE <0|1|2|3>

Obecnie nie uzywany. We wczesniejszych wersjach (do 1.36) decydowal o
sposobie analizy adresow hierarchicznych.

\\HASLO

Uzytkownicy skrzynki moga wprowadzic haslo dostepu zabezpieczajace
wlasne skrytki przed dostepem osob niepozadanych, o ile pozwolil na to
operator skrzynki. Dla lacznosci w protokolach HTTP i POP3 haslem jest 
imie uzytkownika. Dla dostepu przez sieci kablowe jest to haslo TTY.
Zestawienie hasel i algorytmow:

 Dostep               Algorytmy             Rozkazy
 AX25, interaktywny   BayCom,MD2,MD5,Priv   a pw, a pwline, a loginpwtype
 AX25, retransmisja   BayCom,MD2,MD5,Priv   a pw, a sfpwtype
 Operator             BayCom,MD2,MD5,Priv   pw,sysop,priv,md2,md5 DOS:
 Modem                haslo TTY             a ttypw (tylko operator)
                                               Linux,Win32:
 Telnet               haslo TTY             a ttypw (tylko operator)
 POP3-pakiet          imie                  a name
 POP3-Internet        haslo TTY             a ttypw (tylko operator)
 SMTP-pakiet          (przedtem POP3)
 SMTP-Internet        (przedtem POP3)
 HTTP-pakiet          imie                  a name
 HTTP-Internet        haslo TTY             a ttypw (tylko operator)

Rozkaz "Priv" (DieBox) moze byc uzyty tylko pod warunkiem istnienia
zbioru "<znak>.bcm" albo priv.bcm .
Patrz takze: HELP ALTER PW, HELP USERSF HASLO i HELP ALTER PWLINE.

\\HEADER=NAGLOWEK

\\HELP

ROZKAZ-------------znaczenie--------------------------+przyklady
ALTER <parametry>  ustawianie/wykaz parametrow.       A F SR1BSZ.SZ.POL.EU
CHECK <zakres>     spis wiadomosci od czasu ostatniego  C 1-20
                   uzycia rozkazu.
DIR <katalog>      spis tresci katalogu.                D OE1KDA 1-5
DIR ...            wystepuje w wielu wariantach,
                   patrz: HELP DIR.
ERASE <numer>      kasowanie wiadomosci.                E OE1KDA 5
FORWARD <numer> <skrzynka> retransmisja wiadomosci.     F OE1KDA 2 SR6BBS
KOPF <nr>          lub HEADER <nr> wyswietlenie naglowka. K OE1KDA 1-
LIST ...           jak DIR, inny format spisu.          L OE1KDA 5-10
LOG                wyciag z logu np. LOG OE1KDA albo    LOG 28.01.98.
PATH <adres>       wyswietla trase retransmisji.        P SR6BBS
PARAMETER          wyswietla zestaow parametrow skrzynki. PAR
PURGE              kasuje wiadomosci w sposob ostateczny. PU
QUIT               rozlaczenie sie ze skrzynka.         Q (Disconnect)
READ <nr>          odczytanie wiadomosci.               R OE1KDA 2-4
REPLY              odpowiedz na odebrana wiadomosc.     REP
SEND <znak>@<adr> <tytul> nadanie wiadomosci.      S OE1KDA@OE1XLR thenet 
SETLIFE <nr> <okres> zmiana czasu magazynowania         SETL OE1KDA 1 365
                   wiadomosci.                            
TALK <znak> <tekst> wiadomosc do stacji polaczonej ze   T OE1KDA czesc...
                   skrzynka.
TRANSFER <nr> <katalog> kopiowanie wiadomosci.         TR OE1KDA 5 SP6EEK
UNERASE <nr>       odzyskanie skasowanej wiadomosci.    UN OE1KDA 7
USERS              lista stacji polaczonych ze skrzynka. U
VERSION            informacja o oprogramowaniu i wyposa- V
                   zeniu.

Dokladniejsze informacje po podaniu HELP <rozkaz> np. HELP DIR,
 HELP ALTER ...
Spis tresci - po podaniu rozkazu HELP INDEX, pelny tekst po podaniu 
- HELP ALL, zestaw rozkazow - HELP ROZKAZY.

\\HOLD 

(tylko sysop)
Syntaktyka: HOLD [-u] <zakres > 

Komplety zdefiniowal poczte, by nie HOLD (nie S&F). Mozesz wypuscic zatrzymana
poczte z opcja "-u". Mozesz spisac wiadomosci wstrzymane z DIR HOLD.

Zauwaz: Ta komenda jest nonsens usally. Lepsze operacje powstrzymania uzycie
"reject.bcm".
Aby wypuscic poczte z zatrzymanej pozycji, opcja "-u" jest dobrym doborem.
Poczty moze tez zostac wypuszczony z "forward -h" komenda.

Zobacz tez: HELP HOLDTIME, HELP DIR HOLD, i HELP REJECT 

\\HTTP

W odroznieniu od innych systemow poslugujacych sie np. skryptami CGI
skrzynka BayCom-Box obsluguje bezposrednio protokol HTTP, co pozwala
na jej interaktywna obsluge. Uzytkownicy medluja sie w tym przypadku 
przy uzyciu wlasnego znaku wywolawczego. Dla protokolu HTTP przeznaczony
jest domyslnie kanal logiczny TCP o numerze 8080. Operator moze wybrac
inny dowolny numer kanalu. W trakcie pierwszej lacznosci HTTP ze skrzynka
wyswietlane jest okno dialogowe, w ktorym nalezy podac znak i haslo
dostepu. Jako haslo dla lacznosci w petli lokalnej (127.0.0.1) albo przez
radio uzywane jest podane uprzednio imie uzytkownika. Zaoszczedza to 
wysilku operatora, ktory w innym przypadku musialby wprowadzac hasla
dostepu. Uzytkownicy, ktorzy wprowadzili wlasne hasla dostepu do skrzynki
w lacznosciach AX.25 nie moga korzystac z protokolu HTTP. Dla dostepu
przez pozostale kanaly uzywane jest haslo dostepu TTY.
Po zakonczeniu tej pierwszej procedury zgloszenia uzytkownik otrzymuje
miniskrypt, ktory automatyzuje nastepne zgloszenia dzieki czemu
uzytkownik nie musi kazdorazowo podawac swojego hasla. Przyjecie
miniskryptu uzaleznione jest od konfiguracji przegladarki internetowej
(punkt: "cookie").
Jezeli przyjmowanie skryptow jest niedozwolone uzytkownik musi za kazdym
razem odbywac pelna procedure zgloszenia. Zalecane jest korzystanie z
przegladarek interpretujacych ramki (ang. frame) np. przegladarki
"Netscape".
Po uzyskaniu polaczenia uzytkownik moze korzystac ze wszystkich rozkazow
skrzynki. Nalezy zwrocic uwage na nastepujace roznice:
* przed nadaniem wiadomosci nalezy nacisnac przycisk nadawania na ekranie
  Nie wystarczy samo podanie rozkazu nadawania (send) i nacisniecie
  przycisku "wykonaj" ("execute").
* nie mozna wywolywac dodatkowych programow (ang. run utility)
  interaktywnych. 
* odbierane wiadomosci dwojkowe sa automatycznie dekodowane w zaleznosci
  od typu (ang. mimetype). Przykladowo ilustracje w formatach JPG sa
  automatycznie wyswietlane na ekranie.
Pozostale rozkazy sa identyczne jak w przypadku polaczen AX.25.

Po pomyslnym zarejestrowaniu sie serwer HTTP BCM ustawia maly cookie w
prawach uzytkowania webbrowser. Cookie jest uzyty, by zidentyfikowac
prawo uzytkowania dla pozniejszych, zarejestrowan sie i tak do pytanie
zarejestrowania sie bypassu. Jesli webbrowser jest zabroniony, by
zapiswac cookie, prawo uzytkowania musza zarejestrowanie sie z jego
wywolaniem i przekazaniem sterowania prawa uzytkowania i hasla za kazdym
razem w serwerze HTTP BCM.

Webbrowser musi miec ramki; produkty netscape sa dobrym doborem.

Wszystkie funkcje BCM moga zostac zrobione przez interfejs HTTP, ale tam
sa jakies ograniczenia:
- jesli wysylajac mail, ktorych musisz dodac everytime wzajemnej
  zaleznosci.
  To nie pracuje do napisz wyslij komenda w wierszu zlecenia i wtedy,
  kliknac dalej wykonac
- nie mozesz uzyc konwersacyjnego runutils
- jesli przeczytales mail z dwuskladnikowa zawartoscia,(BIN) zawartosc
  jest przemieniona automatycznie (np. JPG pokaze)

Wszystkie inne funkcje powinny pracowac normalnie.
Zobacz tez HRELP HTTPNEWBIE

\\HTTPACCOUNT

(tylko sysop, init.bcm, tylko Linux/Win32)
Syntaktyka: HTTPACCOUNT [ 0 | 1 ]
(wartosc domyslna: 0)

Jesli HTTPACCOUNT jest ustawiony na "1", funkcja "Utworz sobie konto"
jest aktywna. Jesli wpisane jest guestcall, Ty zarejestrujesz sie z tym
guestcall i Twoim IP - numer innym niz "44.xx".
Jesli ta opcja jest zalaczona, guestcall moze utworzyc sobie konto i
haslo ttypw dla znaku, jesli haslo dla tego wpisanego znaku nie jest
ustawiony juz wczesniej.

Wartosc standardowa jest "0" i bedzie nie zmieniona, az sysop chce ta
specjalna mozliwosc zalaczyc.

Zobacz tez HELP GUESTCALL

\\HTTPGUESTFIRST

(tylko sysop, init.bcm, tylko Linux/Win32)
Syntaktyka: HTTPGUESTFIRST [ 0 | 1 ]
(wartosc domyslna: 0)

Jesli HTTPGUESTFIRST jest ustawiony na "1" i sysop zdefiniowal guestcall,
kazdy dostep do http jest automatyczny wykonany jako uzytkownik
"guestcall".
Ten uzytkownik ma tylko dostep do odczytu mail z mailboxa.
Jesli funkcja "userlogin" jest zalaczona to daje mozliwosc do zarejestro-
wania sie z innym jeszcze callsign.

Wartosc standardowa jest "0" i jest niezmienialna, az sysop chce to
specjalna mozliwosc zalaczyc.

\\HTTPLOG

Syntaktyka: HTTPL(OG) ["<lancuch_znakow_przeszukiwania>"]

Przedstawia wszystkich linie pliku "trace/httplog.bcm" ktore zawieraja
przeszukiwany lancuch znakow. Musisz tylko uzyc " " jesli:
 <lancuch_znakow_przeszukiwania> powinien zawierac obszar.
Jesli nie podasz <lancuch_znakow_przeszukiwania> ostatniego 2 kBytes
pliku jest przedstawione.
<lancuch_znakow_przeszukiwania> moze byc data albo znakiem.

\\HTTPNEWBIES

Male streszczenie pomocy dla 'zoltodziobow websurface' mailboxa OpenBCM

Menu na lewo okresla co nastepuje:

* KOMENDY
                 - W ramce mozna wpisac wszystkie rodzaje komend Mailboxa
		   i beda wykonane, jesli po wpisaniu komendy wcisniesz
		   "ENTER".
- Home           = domowe
                  Powrocisz do poczatku, tak jakbys sie logowal odnowa,
                  pokaze Ci sie (polaczeniowy ctekst).

* UZYTKOWNIK
- Userlogin      (tylko, jesli Ty aktualnie jestes zalogowany jako GUEST)
                  Kliknac tutaj, mozesz sie zalogowac swoim wlasnym znakiem
                  i wlasnym haslem.
- Create account = Utworz swoje konto
                 (tylko, jesli sysop ustawil httpaccount 1 w init.bcm)
                  Mozesz utworzyc wlasne konto http przez klikniecie tutaj,
                  jesli ten element danych jest nie widoczny w menu, musisz
                  poprosic sysop o zalaozenie Ci konta.
- Usun cookie
                  Usuwa cookie z programu przegladarki twojego PC-ta,
                  w ktorym programem przegladania zalogowales sie swoim
                  znakiem i swoim haslem.

* POCZTA
- Twoja poczta
                 (tylko, jesli zalogowales sie jako uprawniony uzytkownik)
                  Pokaze Ci ostatnich 20 maili Twojej poczty prywatnej.
- Send mail to  = Wyslij mail do sysopa 
sysop             (tylko, jesli Ty jestes zalogowany jako GUEST).
                  Wysyla mail do sysopa tego Mailboxa.
- Wyslij mail
                 (tylko, jesli zalogowales sie jako uprawniony uzytkownik)
                  Wysyla maile do sieci Packiet Radio z twoim znakiem
- Nowe biuletyny
                 (tylko, jesli zalogowales sie jako uprawniony uzytkownik)
                  Pokaze liste wszystkich nowych maili wg. podzialu na
                  katalogi od ostatniego Twojego ustawiania checktime.
- Ustaw checktime
                 (tylko, jesli zalogowales sie jako uprawniony uzytkownik)
                  Ustawia checktime do biezacego czasu.
- Nowe 300 maili
                  Pokaze liste ostatnich, 300 maili wykazu listy check.
- Wykaz dzialow
                  Pokaze alfabetyczna liste wszystkich katalogow glownych
                  i ich podkatalogow.
- Info OpenBCM
                  Pokaze ostatnich 30 maili w katalogu BAYBOX.
                  Tam znajdziesz informacje na temat nowych wersji OpenBCM.

* SYSTEM
- Status logowan
                  Przedstawia wykazy logowan sie do systemu, oraz forwardy.
- Logi systemu
                  Jest widoczny dziennik roznych logow systemowych.
- S&F status
                  Jest widoczna biezaca statystyka wszystkich prowadzonych
                  przez ten mailbox forwardow poczty.
- Nieznany user
                  Pokaze o ile istnieja, wykazy wszystkich nieznanych
                  adresow uzytkownikow, do ktorych mailbox nie moze
                  wyslac poczty.
- Nieznany BBS
                  Pokaze o ile istnieja, wszystkie znieznane adresy
                  najczesciej hierarchiczne do roznych Mailboxow.
- Wykaz procesow
                  Pokaze wszystkie aktualnie czynne procesy Mailboxa.
- Zalogow. uzytk.
                  Pokaze wykaz aktualnie zalogowanych uzytkownikow.
- Wersja programu
                  Informacja o tej wersji mailboxa, i jego ustawien oraz 
                  jakimi opcjami jest uruchomiony, wielkosc HDD i wolne na 
                  nim miejsce, oraz dane dot samego systemu w formie
                  skroconej.

* POMOC
- Indeks pomocy
                  Pokaze alfabatyczny wykaz wszystkich mozliwych hasel
                  pomocy tego systemu, bez wzgledu czy dane haslo jest
                  uzywane przez obecny system operacyjny.
- Dla nowych      = Pomoc dla "zoltodziobow" ;-)
                  Wlasnie czytasz ten tekst, przeznaczony dla tzw. web
		  "zoltodziobow".


\\HTTPROBOT

(tylko sysop, init.bcm, tylko Linux/Win32)
Syntaktyka: HTTPROBOT [ 0 | 1 ]
(wartosc domyslna: 1)

Jesli HTTPROBOT jest ustawiony na "1", webrobots jak googlebot jest
dozwolony, by umieszczac w indeksie mailbox kompletne przez HTTP-Acces
(Dostep). Jesli ustawiony na "0", robot nie jest zalaczony, umieszczanie
w indeksie mailbox.

\\HTTPTTYPW

(tylko sysop, init.bcm, tylko Linux/Win32)
Syntaktyka: HTTPTTYPW [ 0 | 1 ]
(wartosc domyslna: 0)

Jesli HTTPTTYPW jest ustawiony na "1", tylko haslo ttypw obowiazuje dla
http dostep (tez przez amprnet!). Mozesz uzyc tej mozliwosci by zyskaz
upowaznienie hasla dla pw & wstrzymanie mechanizmu.
Zobacz tez  HELP REJECT.

\\HTTP_PORT

(tylko sysop, init.bcm)
Syntaktyka: HT(TP_PORT) [<n>]

Definiuje, ktory nalezy podac chcac dostac sie do odpowiednich zasobow
sieci komputerowej serwera HTTP BCM. 0 wylaczenie portu.

Przyklad: HTTP_PORT 8080

Patrz tez: HELP PORTNUMMER i HELP HTTP

\\IMPDEL

Syntaktyka: IMPDEL 
Powoduje zakonczenia wykonywania skryptu IMP i samoczynne skasowanie go.
W odroznieniu od wersji poprzednich nie potrzeba podawac nazwy zbioru.

\\IMPORT

Syntaktyka: IMPORT <nazwa zbioru>
Powoduje wykonanie rozkazow zawartych w skrypcie IMP, tak jak gdyby
byly one podane z konsoli lub w trakcie polaczenia radiowego.
Przyklad: zawartosc skryptu STAT.IMP
s statisti @ SR1BSZ # 180 statystyka z logu
...... <dane statystyczne> .....
nnnn                  - zakonczenie zbioru.

Wywolanie skryptu:
IMPORT STAT           - podawanie rozszerzenia IMP nie jest konieczne.
Skrypt rozkazowy moze zawierac wywolania dalszych skryptow.

\\INFO

Syntaktyka: INFO
Wywolanie tekstu informacyjnego. Sklada sie on z czesci standardowej i
tekstu znajdujacego sie w zbiorze "info.bcm". Zawartosc zbioru moze byc
dowolnie modyfikowana przez operatora.

\\INFOLIFE

Syntaktyka: INFOLIFE <dni>

Ustala czas przechowywania wiadomosci w katalogach tymczasowych,
zakladanych w trakcie retransmisji.
Katalogi te zakladane sa w katalogu TMP. Czas ten nie powinien byc zbyt
dlugi w celu ograniczenia liczby katalogow i ulatwienia orientacji.
Katalogi tymczasowe sa kasowane po skasowaniu zawartych w nich wiadomosci.
Czas przechowywania podany przez nadawce wiadomosci jest ograniczony do
podwojnej wartosci podanej przez operatora. 
Uwaga: katalog TMP musi byc uprzednio zalozony przez operatora za pomoca
rozkazu MKBOARD /TMP.

\\INFOPATH

Syntaktyka: INFOPATH <sciezka dostepu>

Podaje sciezke dostepu do katalogow zawierajacych biuletyny.
Wartoscia domyslna jest INFO. Zamiast ukosnika [ \ ] (w stylu DOS) nalezy
uzywac znaku dzielenia  [ / ].
Przyklad: infopath d:/info

Zalecane jest umieszczenie katalogow z biuletynami na oddzielnym dysku
logicznym lub fizycznym (moze to byc tez dysk dostepny przez siec lokalna)
ze wzgledu na znaczna ilosc wiadomosci i ograniczenie przez DOS maksymal-
nej liczby zbiorow na dysku do 2^16. Wiadomosci (biuletyny) zapisywane sa
w oddzielnych zbiorach. Minimalna ilosc zajetego miesca rowna sie
rozmiarowi grupy sektorow (ang. cluster) i jest zalezna od pojemnosci
dysku:
pojemnosc do 256 MB -  4 kB
           256  -  512 MB -  8 kB
           512  - 1024 MB - 16 kB
          1024  - 2048 MB - 32 kB
          2048  - 4096 MB - 64 kB.
Korzystne jest umieszczenie katalogow na dysku sieciowym zarzadzanym
przez inny system operacyjny (systemy zbiorow Novell, Linux - ext2fs,
Windows-NT) ze wzgledu na lepsze wykorzystanie miejsca. 
Dla ext2fs nalezy ustawic wielkosc 1 lub 2 kB.

\\INPUT=OBSLUGA

\\INTERNETMAILGATE

(tylko sysop, init.bcm, tylko Linux / Win32, wybrac INETMAILGATE w config.h)
Syntaktyka: INTERNETMAILGATE [ <CALL> | off ]
(wartosc domyslna: off)

Jesli <CALL> jest zdefiniowany i email jest otrzymany przez interfejs SMTP,
nastepnie zdarza sie:

- adres emailowy nadawcy bedzie przemieniony w adres lokalny <CALL> 
  (np. meier@darc.de -> DB0ABC jesli "internetmailgate db0abc" jest
  zdefiniowany)
- oryginalny adres e-mail nadawcy bedzie dodany do subject poczty 
  (np. "meier@darc.de - blabla" bedzie subject)
- adres e-mail odbiorczy bedzie sciety wierzcholek do znaku odbiorcy 
  (np. db1abc@db0abc.de -> DB1ABC)

Porada: Home-BBS <CALL> powinien byc wlasna skrzynka poczty elektronicznej!

\\JEZYK=ALTER.SPEECH

Patrz tez: ALTER

\\KANALY

W wersjach Linux/Win32 mozna wywolac przyporzadkowanie kanalow
za pomoca rozkazu "info".
Przyporzadkowanie kanalow nastepuje przy uzyciu rozkazow:
SMTP_PORT, POP3_PORT, NNTP_PORT,HTTP_PORT, TELNET_PORT i SERV_PORT.
Numery domyslne (znane powszechnie):
 8025  SMTP
 8110  POP3
 4719  Telnet
 8080  HTTP
    0  NNTP (wylaczenie)
    0  Serwer (wylaczenie)
Zmienione kanaly obowiazuja dopiero po ponownym starcie programu.

\\KASOWANIE=ERASE

\\KATALOG

Oznacza obszar zawierajacy zbiory danych, w skrzynkach elektronicznych
jest swego rodzaju tablica ogloszen. Katalogi sluza do grupowania danych
o zblizonej tematyce w celu ulatwienia orientacji.
Rozrozniane sa nastepujace rodzaje katalogow:
- skrytki:  prywatne katalogi poszczegolnych uzytkownikow
            (ang.user boards). Wiadomosci w nich zamieszczone moga byc
	    kasowane tylko przez nadawce, adresata i operatora skrzynki.
- tablice ogloszen:katalogi ogolnie dostepne (ang. bulletin boards).
            Wiadomosci (biuletyny) w nich zawarte przeznaczone sa dla
            wszystkich. Moga one byc kasowane jedynie przez nadawce i
            operatora skrzynki.
Podobnie jak w systemie operacyjnym DOS, kazdy z katalogow nosi
jednoznaczna nazwe i moze zawerac podkatalogi. W nazwach katalogow
skrytek stosowane sa znaki wywolawcze adresatow. Ogolne nazwy katalogow
moga zawierac litery, cyfry, znak podkreslenia (_) i kreske laczaca (-).
Dopuszczalna dlugosc nazwy wynosi 8 znakow, w programie rozroznianych
jest jednak tylko pierwszych 6. Duze i male litery nie sa rozrozniane.
Wiadomosci zawarte w katalogach nosza kolejne numery w porzadku
chronologicznym. Kazdy z katalogow ma przypisany przez operatora czas
przechowywania wiadomosci. Po jego uplywie wiadomosci sa kasowane
automatycznie. 
Zmiana uzywanego katalogu moze byc dokonana za pomoca rozkazu CD lub
rozkazow DIR, LIST, READ, CHECK i ERASE z podaniem nazwy pozadanego
katalogu. Katalogi o nazwie skladajacej sie z pojedynczej litery maja
szczegolne znaczenie.
Katalogi o nazwach AUTO7P, REQ*, 7PSERV, FILSRV, FNDFIL i WWCALL
sa traktowane jak skrytki prywatne.

\\KILL

Syntaktyka: KILL <numer procesu>

Zakonczenie pracy podanego procesu. Numery czynnych procesow podawane
sa w odpowiedzi na rozkaz PS. Zakonczenie pracy procesu zwiazanego z
polaczeniem radiowym powoduje przerwanie polaczenia. Przerwanie procesu
systemowego moze spowodowac zawieszenie sie programu. Jak widac, osoby
slabiej zorientowane w pracy programu moga wyrzadzic znaczne szkody.

\\KOPF=NAGLOWEK

\\LF=ALTER.LF

\\LINEFEED=ALTER.LF

\\LIFETIME

Syntaktyka: LIFETIME <katalog> <czas normalny> [<czas minimalny>]

Ustala czas magazynowania wiadomosci, maksymalnie 999 dni. Zero powoduje
kasowanie w najblizszym terminie. Czas 999 dni oznacza nieograniczony
czasmagazynowania.
Zmiana czasu magazynowania przez operatora dotyczy w pierwszym
rzedzie nowo naplywajacych wiadomosci. Korekcja czasu dla wiadomosci
znajdujacych sie juz w skrzynce nastepuje po podaniu rozkazu REORG. 
Zmiany czasu sa protokolowane w zbiorze BULLETIN.BCM. Czas magazynowania 
podany przez nadawce wiadomosci jest ograniczany do maksimum podwojnego 
czasu normalnego. Podanie czasu minimalnego powoduje, ze wiadomosci nie 
moga byc skasowane wczesniej niezaleznie od ustalen nadawcy i wplywu 
opoznien w retransmisji. Wartoscia domyslna czasu minimalnego jest 0. 
Operator moze jednak zawsze podac czas nizszy od minimalnego.

\\LIST

Syntaktyka: LIST <nazwa katalogu> <zakres>

Wyswietla spis tresci wybranego katalogu. Format listy rozni sie od
formatu dla rozkazu DIR (patrz pod HELP OPTION).
Rozkaz ten odpowiada rozkazowi LIST oprogramowania skrzynek DieBox.
W spisie wymienione sa wiadomosci, ktore naplynely od czasu ostatniego
polaczenia ze skrzynka (patrz tez pod HELP ALTER).
Przyklady:
l oe1kda 1-   spis wszystkich wiadomosci dla OE1KDA
l baycom -10  spis ostatnich 10 wiadomosci w katalogu baycom

\\LOG

Syntaktyka: LOG [-argumenty] [<data>] [<haslo>]

Powoduje wyswietlenie dziennika stacji dla danego dnia lub wyszukanie
wpisow zawierajacych podane haslo (moze to byc znak stacji lub data).
Wyswietlanych jest ostatnich 20 wpisow.
W rozkazie mozna uzyc nastepujacych argumentow:
  -f   wyswietla takze wpisy dotyczace retransmisji (ang. forwarding).
  -g   wyswietla tylko wpisy dotyczace retransmisji wiadomosci.
  -s   wyswietla tylko wpisy lacznosci operatora.
  -u   wyswietla tylko wpisy retransmisji do uzytkownikow.
  -p   wyswietla tylko wpisy dotyczace lacznosci w protokole POP3.
  -e   wyswietla tylko wpisy dotyczace lacznosci w protokole SMTP.
  -t   wyswietla tylko wpisy dotyczace lacznosci w protokole Telnet.
  -w   przeszukuje dziennik z ostatniego tygodnia (7 dni).
  -m   przeszukuje dziennik z ostatniego miesiaca (30 dni).
  -q   przeszukuje dziennik z ostatniego kwartalu (90 dni).
  -y   przeszukuje dziennik z calego roku.
  -c   wyswietla tylko liczbe pasujacych wpisow.
  -a   wyswietla wszystkie pasujace wpisy (ponad 20).
Dozwolone jest uzycie kilku argumentow jednoczesnie.

LOG <znak>  wyswietla aktualny wykaz lacznosci danej stacji LOG OE1KDA
LOG <data> <znak>  wyswietla wykaz lacznosci danej stacji z danego dnia 
                   LOG 27. OE1KDA
LOG <data>  wyswietla wykaz lacznosci z danego dnia         LOG 18.07.98
Przyklady:
  log -y OE1KDA   podaje liste ostatnich 20 lacznosci OE1KDA w ciagu roku
  log -ws 1.10.   podaje liste lacznosci operatora z tygodnia poprzedzaja-
                  cego dzien 1.10.
  log -cmf        podaje liczbe wszystkich lacznosci w ciagu miesiaca

Format dziennika:
Wyciag (maks. 20 wpisow) z dnia 15.07.98:
znak    data     pocz. kon.  bajt.nad.bajt.od. czas F TXFi RXFi
OE1KDA  15.07.98 08:24-08:27    1059      6    0.49     0   0  via OE1XLR
SP6FIG  15.07.98 08:33-08:49    3936     36    2.47 Q   1   0  via SR6BBS
SP6EEK  15.07.98 09:13-09:14     888     24    0.16 S   0   0  via SR1BSZ
SP9WTV  15.07.98 08:25-08:31   28647   3240    1.41 F   4   2  via SP9YAI
46 wpisow.

bajt.nad. liczba bajtow nadanych do uzytkownika lub skrzynki.
bajt.od.  liczba bajtow odebranych od uzytkownika.
czas:     czas zajetosci jednostki centralnej (CPU).
F:        stan: 
          S = lacznosci operatora, 
          Q = zakonczenie rozkazem QUIT,
          F = retransmisja danych (ang. forwarding).
TXFi:     liczba zbiorow odczytanych ze skrzynki.
RXFi:     liczba zbiorow nadanych do skrzynki.

\\LOGIN

Syntaktyka: LOGIN <znak>

Polaczenie sie ze skrzynka pod podanym, zamiast pod wlasnym znakiem.
Rozkaz przydatny dla operatora do sprawdzenia ustawien parametrow
poszczegolnych uzytkownioow skrzynki. 

\\LOGOUT=QUIT

\\LT

(tylko Sysop)
Syntaktyka: SETLIFE katalog 1- # czas_dni

Czas magazynowania wiadomosci.
Jest to maksymalna liczba dni, po uplywie ktorych wiadomosci sa kasowane
automatycznie. Dla wiadomosci ogolnych jest ona ustalana przez operatora
skrzynki. Wlasciciele skrytek moga w pewnych granicach sami dokonywac jej
zmiany.

\\MAIL=MAILSERVER

\\MAILBEACON

(Tylko sysop, init.bcm)
Syntaktyka: MAILBEACON [ 0 | 1 ]

Jesli wartosc MAILBEACON ustawiona na "1", mailbeacon (plik "beachead.bcm")
jest zawsze wysylany.
Natomiast jesli ustawione jest na "0", mailbeacon wysylany jest tylko gdy 
bedzie nowy uzytkownik skrzynki pocztowej.

Wartosc standartowa to "0" i nie powinna byc zmieniana, chyba ze sysop
postanowi inaczej.

\\MAILFLAGS

W programie przewidziene jest uzycie 16 nowych pol sygnalizacyjnych
informujacych o rodzaju i losie wiadomosci. Narazie uzywane sa 4 z nich:
mailtype: informuje o typie wiadomosci (A, B, P, T)
fwhold:   informuje o przyczynach, dla ktorych wiadomosc nie mogla byc
          przekazana dalej:
  "!" ...wiadomosc zostanie przekazana (pole "Hold" pozostaje puste).
  "R" ...("reject") wiadomosc zostala odrzucona przez skrzynke partnerska
         - meldunek REJ. Oznacza bledna trase.
  "S" ...("size") wiadomosc za dluga (przy podanym parametrze "-b").
  "B" ...("binary") skrzynka partnerska nie obsluguje protokolu Auto-Bin.
  "E" ...("error") blad w zawartosci np. brak uidentyfikatora BID.
  "L" ...("loop") petla na trasie (tylko dla P).
  "N" ...("no") odrzucona przez partnerow nie dysponujacych meldunkami
          REJ.
   W przypadkach roznych od "!" wiadomosc zostaje zatrzymana co jest
sygnalizowane za pomoca pola "Hold".
fwdprop: wiadomosc zostala zaproponowana do retransmisji i zaakceptowana.
replied: odpowiedz udzielona na wiadomosc prywatna.

\\LISTY.ADRESOWE=MAILLISTSERV

\\MAILLISTSENDER 

(tylko sysop, init.bcm, mozliwosc ustawien MAILSERVER w config.h)
Syntaktyka: MAILLISTSENDER [ 0 | 1 ]
(wartosc domyslna: 1)

Ten parametr definiuje znak nadawcy poczty wysylanej. 
Ta wartosc powinna zostac ustawiona na "1", systemy FBB BBS otherways
moze nauczyc sie niepoprawnego MYBBS od uzytkownika, ktory uzywa listu
wysylanego.
maillistsender 0 oryginalny nadawca jest uzyty jako nadawca dla listu
                 wysylanego (uzyj tego tylko, jesli poczta nie podaje FBB
		 BBS) 
maillistsender 1 znak skrzynki pocztowej jest uzyty jako nadawca dla
                 wysylanej poczty 

\\MAILLISTSERV

Syntaktyka: MAILLISTSERV 0|1|2

Decyduje o mozliwosci wprowadzania list adresowych:
0 - usluga wylaczona,
1 - usluga wlaczona,
2 - usluga wlaczona ale tylko operator moze wprowadzac nowe listy.

\\MAILSERVER

Sluzy do rozprowadzania prywatnych wiadomosci do ustalonego wczesniej
grona adresatow. W celu rozprowadzenia wiadomosci nalezy wyslac ja na
adres skrzynki. Wywolanie uslugi nastepuje w linii tytulowej:
"mailto <serwer> tytul".
Patrz tez: HELP LISTY ADRESOWE.

Konfiguracja uslugi dokonywana jest za pomoca nastepujacych rozkazow:
HELP MAILServer +L
HELP MAILServer -L
HELP MAILServer DESCRIPTION
HELP MAILServer OPTIONS
HELP MAILServer +U
HELP MAILServer -U
HELP MAILServer +M
HELP MAILServer -M
HELP MAILServer Info
HELP MAILServer RESET
HELP MAILServer SUBSCRIBE
HELP MAILServer UNSUBSCRIBE
HELP MAILServer SETNUMBER

\\MAILSERVER.ADDMAIN=MAILSERVER.+M

\\MAILSERVER.DELMAIN=MAILSERVER.-M

\\MAILSERVER.ADDUSER=MAILSERVER.+U

\\MAILSERVER.DELUSER=MAILSERVER.-U

\\MAILSERVER.NEWLIST=MAILSERVER.+L

\\MAILSERVER.DELLIST=MALISERVER.-L

\\MAILSERVER.HELP=MAILSERVER

\\MAILSERVER.SETNUMB

Syntaktyka: MAILServer SETNUMBER <n>
Podaje numer nastepnej wiadomosci.

\\MAILSERVER.+

Syntaktyka: MAILSserver +M <lista adresowa> <znak>

Dodaje administratora do wymienionej listy adresowej. 
Zakladajacy liste otrzymuje automatycznie uprawnienia administratora.

\\MAILSERVER.+U

Skladnie: MAILServer +U <lista adresowa> <znak>

Nanosi podany znak na liste adresowa.

\\MAILSERVER.-L

Syntaktyka: MAILServer -L <lista adresowa>

Kasuje podana liste adresowa.

\\MAILSERVER.-M

Syntaktyka: MAILServer -M <lista adresowa> <znak>

Usuwa administratora listy.

\\MAILSERVER.-U

Syntaktyka: MAILServer -U <lista adresowa> <znak>

Usuwa z listy podana stacje - adresata.

\\MAILSERVER.DESCRI

Syntaktyka: MAILServer DEscription <lista adresowa> <opis>

Sluzy do wprowadzenia krotkiego opisu dotyczacego listy.
Maksymalna dlugosc wynosi 60 znakow.

\\MAILSERVER.INFO

Syntaktyka: MAILServer Info <lista adrersowa>

Wywoluje informacje dotyczace podanej listy adresowej.
Opuszczenie nazwy powoduje wyswietlenie spisu list.

\\MAILSERVER.LIST=MAILSERVER.INFO

\\MAILSERVER.+L

Syntaktyka: MAILServer + <lista adresowa>

Sluzy do zalozenia nowej listy.

\\MAILSERVER.OPTION

Syntaktyka: MAILServer OPTION <lista adresowa> <parametr>

Sluzy do konfiguracji listy adresowej. Znaczenie parametrow:
-c tylko administratotrzy moga dodawac/usuwac adresatow
-m tylko administratorzy moga nadawac wiadomosci do uczestnikow.
-u tylko uczestnikom wolno rozpowszechniac wiadomosci do
   pozostalych adresatow.
   Rozkaz bez parametru kasuje poprzednio ustawiony warunek.

\\MAILSERVER.RESET

Syntaktyka: MAILServer RESET <lista adresowa>

Licznik poczty zostaje ustawiony na 1.
           
\\MAILSERVER.SUBSCR

Syntaktyka: MAILServer SUBSCRIBE <lista adresowa>

Wpisanie sie na liste adresowa. Alternatywa jest wyslanie na adres
skrzynki wiadomosci o nastepujacym tytule "SUBSCRIBE <lista adresowa>"

\\MAILSERVER.UNSUBS

Syntaktyka: MAILServer UNSUBSCRIBE <lista adresowa>

Sluzy do usuniecia wlasnego znaku z listy. Alternatywa jest wyslanie na
adres skrzynki wiadomosci o tytule "UNSUBSCRIBE <lista adresowa>".

\\MAKROS=METASYMBOL

\\MAKRO=METASYMBOL

\\METASYMBOL

Jest symbolem tekstu lub wyrazenia uzywanym w zgloszeniu skrzynki. 
Jest on zastepowany przez odpowiednie wyrazenie.
W tekscie zgloszenia oraz w tekstach powitalnych i informacyjnych
(CTEXT/CNEW/INFO) moga wystepowac nastepujace metasymbole:
  %a   obciazenie jednostki centralnej w % (w ciagu ostatnich 40 sekund).
  %b   nazwa uzywanego katalogu.
  %c   znak uzytkownika skrzynki.
  %d   biezaca data.
  %h   rodzaj wyswietlanych tekstow pomocy.
  %i   czas poczatku lacznosci.
  %l   data i czas poprzedniej lacznosci.
  %m   znak wywolawczy skrzynki.
  %n   imie uzytkownika.
  %o   aktualna liczba uzytkownikow.
  %p   czas pracy jednostki centralnej od poczatku polaczenia.
  %r   koniec linii (znak return).
  %s   wolny obszar pamieci w kB.
  %t   aktualny czas
  %u   czas pracy od ostatniego wlaczenia skrzynki.
  %v   numer wersji oprogramowania
  %w   liczba odczytanych/nadanych bajtow.
  %%   znak %

Przyklad:   
a pr [%o}(%b) %c de %m => zgloszenie w stylu skrzynek DieBox 1.9
                          (BAYCOM) OE1KDA de SR6BBS>
albo        
a pr [%o/%a%%](%b)-->     liczba uzytkownikow i obciazenie CPU
     [22/95%](OE1KDA)-->

\\MAN=HELP

\\MAXBIDS

Syntaktyka: MAXBIDS <liczba>

Ustala maksymalna liczbe identyfikatorow biuletynow (BID). Powinna ona
byc wieksza od przewidywanej maksymalnej liczby przechowywanych
biuletynow.
Po jej przekroczeniu dokonywana jest reorganizacja wewnetrznego spisu
biuletynow. Zalecana wartoscia jest 65536 (DOS) lub 150000 (LINUX).
Zbyt duza liczba powoduje przedluzenie czasu poszukiwania i spowolnienie 
pracy skrzynki.

\\MAXLOGINS

Syntaktyka: MAXLOGINS <liczba>

Ustala maksymalna liczbe rownoleglych polaczen ze skrzynka dla kazdego
z uzytkownikow (poza operatorem i kanalami retransmisji). Zero oznacza
brak ograniczen. Ograniczenie liczby polaczen moze zmniejszyc prawdopodo-
bienstwo przeciazenia skrzynki. 

\\MEM

Syntaktyka: MEM (rozkaz ma znaczenie dla programisty lub operatora skrzynki).

Wyswietlenie informacji technicznych odnoszacych sie do zajetosci
pamieci i czynnych procesow programu. Wyswietlane sa:
- biezacy numer bloku pamieci.
- rozmiar bloku w bajtach (maks. 64 kB).
- adres (segment) bloku.
- numer procesu (patrz HELP PS) lub 255 w przypadkach specjalnych.
- rodzaj danych.

\\MD2

Jest to algorytm obliczania sumy kontrolnej. Jest ona obliczana w oparciu 
o tekst znany obu stronom i wygenerowany przez skrzynke ciag cyfr losowych
Skrzynka generuje w oparciu o tekst hasla ciag cyfr i nadaje go do
uzytkownika.
Program terminalowy uzytkownika generuje sume kontrolna z tego samego
hasla, do ktorego n akoncu doczepiony zostal odebrany ciag. Ciag ten
przekazywany jest do skrzynki, ktora powtarza obliczenia i porownuje
wyniki. W przypadku ich zgodnosci uzyskuje dostep. Patrztez: HELP HASLO.

\\MD5=MD2

\\MK=MKBOARD

\\MKBOARD

Syntaktyka: MK <katalog nadrzedny> <nowy katalog>

Sluzy do zalozenia nowego katalogu w podanej i istniejacej juz katalogu.
Rubryka - katalogiem - nadrzedna moze byc takze poczatek ukladu (/).
Przyklady: 
MKBOARD software baycom  - zaklada katalog w katalogu glownym SOFTWARE
MKBOARD / software.      - zaklada katalog glowny SOFTWARE

Dla kazdej z rubryk nalezy nastepnie okreslic czas przechowywania
wiadomosci, domyslnie przyjmowana jest wartosc podana za pomoca rozkazu
INFOLIFE.
Zalozony katalog jest automatycznie rejestrowany w zbiorze BULLETIN.BCM.

\\MONITOR

Syntaktyka: MONITOR [-<argumenty>] <numer procesu>

Pozwala na obejrzenie danych wejsciowych i wyjsciowych dowolnego procesu
systemowego albo zwiazanego z kanalem lacznosci. Numery procesow zawarte
sa w spisie PS. Szczegolnie interesujace moga byc dane zwiazane z oknami
przebiegu pracy (ang. trace, nr. 1) i monitora (nr. 3). 
W oknach tych nie jest wyswiertlany ostatni znak RETURN (ze wzgledu na
brak miejsca).
Argumenty:
-i ignorowanie danych wprowadzanych przez uzytkownika.
-o ignorowanie danych nadawanych do uzytkownika.
-r ignorowanie zawartosci wiadomosci (odczytywanych za pomoca rozkazu
   READ).
-d ignorowanie spisow wywolywanych za pomoca DIR/LIST/CHECK.
-s ignorowanie tresci wiadomosci nadawanych za pmoca rozkazu SEND.
Dla kazdego z procesow mozna wywolac monitor tylko raz.

\\MSG=TALK

\\MV=MVBOARD

\\MVBOARD

Syntaktyka: MVBOARD <rubryka_gl._zrodlowa> <rubryka_podrzedna> <docelowa_rubryka gl.>

Przesuwa zawartosc podanej rubryki podrzednej do nowej nadrzednej rubryki 
docelowej bez zmiany nazwy rubryki. 
Przyklady: 
1. - wiadomosci zawarte w TMP/C64 przenoszone do COMPUTER/C64
MV TMP C64 COMPUTER

2. - wiadomosci zawarte w /C64 przenoszone do COMPUTER/C64
MV / C64 COMPUTER

3. - wiadomosci zawarte w TMP/C64 przenoszone do /C64
MV TMP C64 /

Alternatywna mozliwosc przeniesienia za pomoca rozkazu TRANSFER
wymagalaby zmiany nazwy rubryki i jest wolniejsza. Dla nowej rubryki
przyjmowana jest domyslna wartosc czasu magazynowania.

\\MYBBS=ALTER.FORWARD

\\MYCALL

Syntaktyka: MYCALL <znak1> [<znak2>] [<znak3>] [<znak4>] [<znak5>] [<znak6>] ..

Podaje znaki wywolawcze skrzynki. W trakcie retransmisji uzywany jest
pierwszy z podanych znakow z dodatkiem SSID ustalonego za pomoca rozkazu
FWDSSID lub o jeden wyzszego (patrz: HELP FWDSSID).
Przyklady:
 MYCALL SR1BSZ-8
 FWDSSID 9
oznacza, ze skrzynka dostepna jest dla wszystkich pod znakiem SR1BSZ-8,
a w czasie retransmisji uzywane sa znaki SR1BSZ-9 i SR1BSZ-10.
 MYCALL SR1BBS SR1BSZ-8
 FWDSSID 9
 - oznacza, ze skrzynka dostepna jest dla wszystkich pod znakami
   SR1BBS-0 i SR1BSZ-8, a w czasie retransmisji uzywane sa SR1BBS-9 i 
   SR1BBS-10 pod warunkiem prawidlowego zadeklarowania lacza w programie
             wezla.

Szczegolne znaczenie kolejnych znakow wywolawczych:
<znak1>   - dla wszystkich polaczen z wezlem,
<znak2>   - dla retransmisji z prywatnymi skrzynkami uzytkownikow
<znak3>   - polaczenia bez retransmisji poczty,
<znak4>   - zwykly znak skrzynki,
pozostale - wystepuja tylko w skryptach rozkazowych w wersji dla systemu
            LINUX.

\\NAGLOWEK

KOPF <katalog> <zakres> albo HEADER <katalog> <zakres>
Wyswietla naglowki wybranych wiadomosci (zbiorow). 
Skladnia jest identyczna jak dla rozkazu READ.
Po podaniu argumentu -h wyswietlana jest trasa retransmisji wiadomosci
(zawartosc linii R: ).

\\NAME=ALTER.NAME

\\NE=NEW

\\NEW

(tylko sysop)
Syntaktyka: NE(W)

Wczytanie zmian plikow, bedzie wykonana, jesli jakis plik jest zmieniony:
 init.bcm       glowny plik konfiguracji systemu 
 beacon.bcm     adresy dla pakietow radiolatarni 
 bulletin.bcm   katalogi i czasy ich magazynowania
 convert.bcm    konwersja katalogow i czasow magazynowania 
 fwd.bcm        trasy retransmisji
 afwd.bcm       autoforward (jesli istnieje)
 macro.bcm      zestaw makrorozkazow 
 msg/messages.* meldunki skrzynki (w roznych jezykach) 

\\NEXT

Syntaktyka: NEXT [-R]

Sluzy do odczytania nastepnej wiadomosci z katalogu roboczego.
Parametr -R powoduje odczytanie poprzedniej wiadomosci.

\\NH=ALTER.FORWARD

\\NNTP_PORT

Syntaktyka: nntp_port <kanal>

Wybor kanalu logicznego TCP dla NNTP.
Przyklad: nntp_port 0
(0 oznacza wylaczenie kanalu)
Patrz tez:  HELP KANALY

\\NOPURGE

Syntaktyka: NOPURGE 0|1

Pozwala na calkowite wylaczenie fizycznego kasowania zbiorow (1).
Patrz tez: HELP PURGE ARGUMENTY.
Dostepne sa tez dodatkowe parametry:
2 - pozwala uzytkownikom na indywidualne korzystanie z rozkazu:
    NOPURGE 0|1
4 - biuletyny nie sa kasowane a jedynie zaznaczane i niewidoczne dla
    uzytkownikow. Mozna je odzyskac za pomoca rozkazu UNERASE.
8 - jak 4 tylko dla wiadomosci prywatnych.
Parametry te moga byc kombinowane np. dla systemow o wystarczajaco duzej
pojemnosci twardego dysku sensowne jest podanie  "nopurge 6".
Patrz tez: HELP PURGE ROZKAZY, HELP ALTER UNERASE, HELP PURGE
i HELP PURGE A

\\NOTE

Syntaktyka: NOTE <tekst>

Umozliwia wpisanie tekstu do dziennika czynnosci operatora. Tekst
rozpoczyna sie slowem "note" i jest zaznaczony symbolem #L.

\\OBSLUGA

Rozkazy dla skrzynki musza byc zakonczone znakiem LF (nalezy nacisnac
klawisz RETURN/ENTER). Linia rozkazowa moze zawierac kilka polecen
oddzielonych srednikiem, np: DIR TNC -20;R 3-4.
W celu przerwania transmisji wiadomosci tekstowej ze skrzynki nalezy
nadac pusta linie. Transmisja zbiorow dwojkowych nie moze byc przerwana.
Uzycie kilku blednych rozkazow pod rzad powoduje przerwanie polaczenia
przez skrzynke. Moze to sie zdarzyc jezeli programy terminalowe nadaja
automatycznie dodatkowe informacje do korespondenta, np. po zakonczeniu
transmisji zbioru. W niektorych z nich przewidziana jest mozliwosc 
zadeklarowania korespondenta jako skrzynki elektronicznej w celu 
wylaczenia dodatkowych meldunkow.

\\ODSTEP=ALTER.LF

\\OLDESTFWD

Syntaktyka: OLDESTFWD <dni>

Okresla maksymalny wiek retransmitowanych wiadomosci. Starsze wiadomosci
nie sa retransmitowane. Wartoscia domyslna (i zalecana) jest 30 dni.
Wartosc zero jest zamieniana na 999 dni.

\\OMD

Syntaktyka: omd <nazwa katalogu|sciezka>

Zalozenie nowego katalogu lub calego drzewa katalogow.
Przyklad: omd c:\bcm\test\neu\abc
Patrz tez: OMV, ORM

\\OMV

Syntaktyka: omv <nazwa> <nowa nazwa>

Sluzy do zmiany nazwy zbioru.
Patrz tez: ORM, OMD

\\OPCJE.FORWARD

To sa wszystkie mozliwe opcje S&F:
-b <bajt> rozmiar maksymalny forward mails ( > 10k)
-d (delay) S&F tylko w o ustawionych czasach w (crontab.bcm)
-e wysyla E/M pliki z 4 liniami w 1 ramce (e.g. DB0SAO)
-f wysyla pusta linie przed uruchomieniem S&F (e.g. xNOS TCP/IP)
-i mails BoxBin nie sa transmitowane baczac na capatybilnosc z
   odbierajacym systemem (nie przydatny w wiekszosci wypadkow)
-k no forward biuletynu "autobin"
-l no forward "autobin" maili uzytkownika
-m no forward "7plus" biuletyn
-n no forward "7plus" maili uzytkowanika
-o zapisy sa sortowane wzgledem rozmiaru (najmniejszej najpierw)
-p ukrywa prompt po logowaniu sie (e.g. potrzebne dla DB0SAO)
-r wysyla TYLKO biuletyny wpisane  w "fwd.bcm" do S&F partnera
-s SID jest wyslany natychmiast po polaczeniu z BBS
-t rejestrowanie transmisji aktywne (jesli ustawiony jest ("fwdtrace 2")
-u polaczenie jest zamkniete natychmiast, bez tego parametru S&F czeka
   przez pewien czas, aby nowe maile mogly byc transmitowane

UWAGA: Powinienes stosowac opcje "-l" w tym przypadku, jesli S&F partner
nie moze obsluzyc 7plus albo maili autobin, jak bramka.
Nie ograniczaj maili uzytkownika bez powodu!
Parametr "-b" ma tylko sens, jesli S&F partner korzysta tylko z ASCII S&F
              protokolu (WA7MBL). Wewnetrznie ograniczony BCM rozmiar do
              jednego maila w jednej chwili do 10 MBytes. Kazda opcja
	      musi byc oddzielona jednym znakiem odstepu, jesli uzywasz
	      wiecej niz jedena opcje.

Zobacz tez: HELP FWDTIMEOUT, HELP FWDTRACE i HELP FWDSSID.

\\OPTION.FORWARD=OPCJE.FORWARD

\\ORM

Syntaktyka: orm <plik>

Sluzy do skasowania zbioru.
Patrz tez: OMV, OMD

\\OSHELL

Syntaktyka: OSHELL <rozkaz dla systemu operacyjnego>

Wykonanie podanego rozkazu systemu operacyjnego. W systemie DOS wymaga
to zaladowania interpretatora rozkazow wskazywanego przez zmienna
srodowiskowa COMSPEC (standardowo COMMAND.COM). Sposob wykonywania
regulowany jest przez parametr rozkazu DOSINPUT. Patrz tez: HELP DOSINPUT
Rozkaz OSHELL bez dodatkowego argumentu powoduje przejscie na poziom
DOS do czasu podania rozkazu EXIT.  
Wywolywane programy lub rozkazy DOS musza:
- przekazywac dane wyjsciowe przez podprogramy DOS/BIOS, a nie wpisywac
  bezosrednio do pamieci wizyjnej.
- dane wejsciowe moga zawierac czytelne znaki ASCII i znaki kontrolne. 
  Kombinacje klawiszy z ALT, klawisze sterujace znacznikiem nie sa
  dozwolone. 
- ze wzgledu na zatrzymanie pracy skrzynki czas wykonywania rozkazu
  powinien byc mozliwie krotki.
- nie wolno w tym przypadku korzystac ze sterownika SHROOM.
Nalezy zrocic tez uwage na mozliwe straty znakow w kanalach wejsciowych
w trakcie wykonywania rozkazow DOS (problem ten wystepuje tylko gdy 
DOSINPUT=1).

\\PARAMETER

Syntaktyka: PARAMETER <haslo>

Wywoluje liste parametrow skrzynki, podanie hasla pozwala na zawezenie
listy do parametrow zawierajacych to haslo. Parametry zawarte sa w
zbiorze INIT.BCM. Przyklad: PAR RUNUTILS wywoluje spis programow
przeznaczonych do zdalnego wywolywania przez uzytkownikow skrzynki.

\\PASSWORD=HASLO

\\PATH

Syntaktyka: PATH <adres>

Podaje trase retransmisji poczty (biuletynow i wiadomosci prywatnych) do 
stacji o podanym znaku. Wymienione sa znaki wszystkich skrzynek
posrednich.
Przyklady:
  p sr6bbs
  p all
  p oe1kda.aut.eu
Patrz takze: HELP PATH SZCZEGOLY.

\\PATH.SZCZEGOLY

Syntaktyka: PATH [-A] [-S][H|P] [-C] <adres>

  p -a <adr>  wyswietla trasy polaczen do podanej stacji pod warunkiem,
              ze trasa byla juz uzywana. 
  Spis zawiera nastepujace pola: 
  H-Addr:     ostatni znany adres hierarchiczny.
  R-Header:   zawartosc linii R w naglowku.
  Last BID:   identyfikator BID ostatniej otrzymanej wiadomosci.
  Last Board: rubryka (katalog) dla ktorej byla przeznaczona ostatnia
              wiadomosc.
  Last User:  adresat lub nadawca wiadomosci.
  Bulletins:  liczba otrzymanych biuletynow.
  Usermails:  liczba otrzymanych wiadomosci prywatnych.
  Jako nastepna nadawana jest statystyka, kiedy, ile wiadomosci, po jakim
  czasie i przez ile skrzynek posrednich otrzymano. 
  Pozwala ona na przesledzenie i optymalizacje tras retransmisji.

  p -s <..> selekcja wiadomosci w oparciu o podane haslo poszukiwan.
       Moze nim byc czesc adresu hierarchicznego, zawartosc linii R:
       lub termin wystepujacy w tresci, np. P - S #JG selekcjonuje
       skrzynki lezace w rejonie JG, P -S BCM poszukuje skrzynek systemu
       BayCom. Wyswietlana jest liczba wiadomosci, czas ostatniej wymiany,
       adres hierarchiczny i zawartosc linii R:  (trasa retransmisji).  

  p -sh <..> selekcjonuje wiadomosci w oparciu o naglowki z pominieciem 
       adresow. 

  p -sp <..> przeszukuje tylko adresy hierarchiczne z pominieciem
       naglowkow, np. P -SP POL wybiera tylko skrzynki polskie. 

  p -sc <..> podaje tylko liczbe znalezionych wpisow. 

\\PATH.EDIT

Syntaktyka: PATH <..>

Pozwala na zmiane wprowadzonych adresow hierarchicznych:
p -ea <adres>              podanie nowego adresu.
p -ed <znak>               skasowanie wpisu.
p -eh <znak> <naglowek>    wprowadzenie nowej zawartosci linii R:.
p -et <znak> <data>        podanie nowej daty odbioru w naglowku. Moze
                           byc takze data z przyszlosci.

\\PCFLEX=FLEXNET

\\PCISUTC

(Tylko dla DOS)
Syntaktyka: PCISUTC 0 | 1

Decyduje o czasie ustawionym na zegarze CMS (UTC lub lokalny).
Czas lokalny przelaczany jest automatycznie na zimowy i letni.
Patrz tez: HELP STIMEOFFSET

\\PING

Syntaktyka: PING <BBS>

Opcja sluzy do wyslania maila do danego (OpenBCM) w celu ustalenia
sciezki jaka poczta pojdzie lub aktualnie idzie do danego BBS oraz czas,
w obu kierunkach.

\\POCZTA
(tylko gdy aktywny jest runutils, skrypt: poczta)
Syntaktyka: poczta list, poczta read

Pokazuje lub czyta poczte ktora znajduje sie na serwerze pocztowym
Linux sp1lop.szczecin.pl

\\POMOC=HELP

\\POP3=SMTP

\\POP3-SMTP-CONF=SMTP-POP3.CONFIG

\\POP3_PORT

Syntaktyka: pop3_port <kanal TCP>

Ustala numer kanalu logicznego TCP dla protokolu POP3.
Przyklad: pop3_port 8110
Patrz tez: HELP KANALY

\\POPMAIL
(tylko gdy aktywne jest runutils, skrypt: popmail)
Syntaktyka: popmail list, popmail read

Pokazuje lub czyta poczte na bramce amprnet JNOS 2.0ePL 
szczecin.ampr.org

\\POSTFWD

Powoduje retransmisje zalezalej (niedostarczonej dotad) poczty.
Moze to byc konieczne w przypadku:
- przerwania polaczenia w trakcie retransmisji.
- przerwania pracy skrzynki.
- nieznajomosci adresu docelowego w momencie otrzymania wiadomosci.
- blednego adresu korespondenta, ale prawidlowego adresu jego skrzynki
  macierzystej.
- blednego adresu i znaku skrzynki w momencie nadejscia poczty,
  skorygowanych na podstawie pozniejszych wiadomosci.
Rozkaz POSTFWD wpisuje ponownie wiadomosci do kolejki wyjsciowej.
Skrzynka powinna byc w regularnych odstepach czasu oczyszczana z 
zalezalej poczty. Najwygodniej zrobic to za pomoca nastepujacego 
wpisu do tabeli CRONTAB.BCM: 
;--------------------
;minute   hour  day month weekday   command
5           2    *    *     *       postfwd
; codziennie o 02:05.
;--------------------

\\POZIOM.POMOCY=ALTER.HELP

\\POZEGNANIE=QUIT

\\PS

Wyswietla liste aktywnych procesow (programow, ang. process status).
Kazdemu z uzytkownikow i kazdemu z otwartych okienek przypisany jest 
oddzielny proces. Oprocz tego czynna jest pewna liczba procesow
systemowych, pracujacych w tle.
Argumenty:   
PS -a   podaje takze procesy pasywne
   -b   podaje procesy pracujace w tle
   -m   podaje takze znak polaczonej stacji (analogicznie jak u *)
   -n   podaje takze imiona uzytkownikow
   -r   podaje takze biezacy katalog
   -s   podaje zajetosc stosu i kolejnosc wywolywania procesow
   -t   podaje numeracje kanalow kotrolerow TNC
   -u   podaje znak wezla wejsciowego
do wywolania wszystkich tych informacji sluzy rozkaz:
ps -abmrstu

Standardowo podawane sa tylko aktywne procesy zwiazane z wymiana
informacji.
Aktywne procesy zaznaczone sa za pomoca znaku * w pierwszej kolumnie.
Przyklad:   ps -abs
Task Event Kdeb  CPUt Create Input Name      Command
   0 40     210   0.1   71m        Desktop
   1 251    210   0.3   71m        Console
   2 242    210   23s   71m        Monitor
   3 246    000   28s   71m        logind
   4 kbhit  711   0.7   71m        Sysop
   5 514    000   0.3   71m        forwardd
   6 235    101   3.9   18m   26s  DC6IU     l ibm
*  7 235    101   81s   71m    0s  DL8MBT    ps -abs
   8 txf    101   9.2   54m   40m  OE5GOL    r baybox 25-28
   9 235    101   8.5   26m    5m  DG1GOB    R CONTEST 88
  12 txf    101   31s   69m  139s  DJ9EI     r gp 218-
  13 242    101   0.8   11m   35s  DG5MGN    rep
Wait:  0 13 9 3 2 1 6 5 12 8 4
Stack: 272 174 446 870 790 530 1108 1080 1158 1720 1204 1720
Poszczegolne rubryki oznaczaja:
Task:     numer procesu, gwiazdka oznacza proces wlasny.
Event:    oczekiwana reakcja z zewnatrz;
  kbhit:  nacisniecie klawisza.
  putf:   oproznienie bufora nadawczego w kontrolerze TNC
  getv:   odbior z kontrolera TNC
  slice:  przerwany z powodu wyczerpania sie przydzielonego czasu.
  <liczba>:czas oczekiwania w milisekundach. Interesujacy do celow
          diagnostycznych.
Kdeb:     typ procesu Kd:zadanie wewnetrzne e:obsluga ekranu
          b:skrzynka el.
CPUt:     czas pracy jednostki centralnej od uruchomienia procesu
Create:   czas zycia procesu
Input:    czas, ktory uplynal od ostatniej komunikacji z uzytkownikiem
Name:     nazwa procesu lub znak uzytkownika
Command:  ostatni podany rozkaz
Wait:     kolejnosc wywolywania procesow
Stack:    maksymalne sumaryczne wykorzystanie stosu

Procesy pracujace stale lub cyklicznie:
Desktop   glowny proces, obsluga ekranu.
Console   okienko zawierajace meldunki systemu, meldunki bledow wpisywane
          sa dodatkowo do zbioru SYSLOG.BCM.
Monitor   okienko do podgladu komunikacji packet-radio.
Sysop     okienko dla operatora systemu.
logind    proces odpytujacy cyklicznie wszystkie kanaly logiczne
          kontrolerow TNC w celu stwierdzenia nawiazania lub przerwania
	  polaczen (ang. login-daemon). W miare potrzeby uruchamia
	  procesy obslugujace uzytkownikow.
forwardd  proces obslugujacy cyklicznie retransmisje danych 
                                               (ang.forward-daemon)
          Prowadzi rejestracje czynnosci w zbiorach BULLETIN.BCM
	  i INIT.BCM.
	  Raz na dobe wywoluje rozkaz PURGE (o godzinie 4).
crond     uruchamianie procesow w zadanych momentach czasu.
          (purge,reorg itp.)
purge     kasuje w sposob ostateczny zbiory o przekroczonym czasie
          magazynowania lub skasowane przez uzytkownikow.
reorg     reorganizuje system zbiorow skrzynki.
postfwd   wpisuje oczekujace wiadomosci prywatne do kolejki zawartej w
          '/bsm/fwd/u_<bbs>.bcm'.
Dla dostepu przez TCP/IP zamiast znakow stacji wezlowych podawane sa
adresy IP.

\\PURGE

Rozkaz ERASE nie powoduje rzeczywistego skasowania wiadomosci, a jedynie
zaznaczenie ich za pomoca specjalnego atrybutu. Do ich rzeczywistego
skasowania sluzy rozkaz PURGE. Wiadomosci tak skasowane nie moga byc
odzyskane za pomoca rozkazu UNERASE. Po skasowaniu ulega zmianie numeracja
pozostalych wiadomosci. Proces PURGE moze byc zablokowany przez operatora
za pomoca rozkazu "nopurge 1".

\\PURGE.A

Syntaktyka: PURGE A

Natychmiastowe wywolanie procesu fizycznego kasowania wiadomosci.
Moze byc przeprowadzone dodatkowo do automatycznego wywolywanego przez 
tabele CRONTAB.BCM. Fizycznie kasowane sa wiadomosci zaznaczone za pomoca
rozkazu ERASE i wiadomosci o przekroczonym czasie skladowania.
Wiadomosci zaznaczone moga byc przed fizycznym skasowaniem odzyskane za
pomoca rozkazu UNERASE. 
Rozkaz PURGE kasuje takze puste rubryki (katalogi).
Proces PURGE moze byc zablokowany przez operatora za pomoca rozkazu
"nopurge 1" 
W zaleznosci od ilosci kasowanych wiadomosci proces moze trwac od kilku
minut do godziny, nie przerywa on jednak pracy skrzynki.
W trakcie kasowania moze dojsc jednak do przedluzenia czasu reakcji
programu. 
Zakonczenie kasowania wpisywane jest do dziennika pracy skrzynki:
mbpurge: 10 era, 20 lt, 234 all
znaczenie pol:  
era:  wiadomosci skasowane za pomoca ERASE i po retransmisji.
lt:   wiadomosci skasowane po przekroczeniu czasu skladowania.
all:  liczba przeszukanych zbiorow.

Bledy w zbiorze LIST.BCM zauwazone w czasie kasowania powoduja jego 
przerwanie i przeprowadzenie reorganizacji (rozkaz REORG). Bledne spisy 
CHECK sa na nowo generowane po zakonczeniu kasowania.
Kasowanie wiadomosci powoduje korekcje ich numeracji w spisie CHECK.
Rozkaz PURGE <rubryka> powoduje tylko skasowanie wiadomosci w podanej 
rubryce. Rozkaz bez parametrow jest dostepny dla wszystkich uzytkownikow
i odnosi sie do ich katalogow prywatnych.
Kazdy z uzytkownikow moze zablokowac kasowanie wlasnych wiadomosci
za pomoca rozkazu ALTER NOPURGE 1. 
Operator moze dokonac tego poslugujac sie rozkazem:
 SETUSER <znak> NOPURGE 0|1
Polecenie odnoszace sie do biuletynow wykonywane jest w tle.
Patrz takze: HELP PURGE ROZKAZY.

\\PURGE.ROZKAZY

Na przebieg procesu kasowania maja wplyw nastepujace rozkazy:
PURGE         - zapoczatkowanie procesu (przewaznie automatycznie w
                oparciu o zawartosc tabeli CRONTAB.BCM).
NOPURGE       - zablokowanie procesu.
ALTER NOPURGE - zablokowanie dla katalogu prywatnego.

\\PURGEHOUR

Syntaktyka: PURGEHOUR <godzina>
Obecnie bez znaczenia. W poprzednich wersjach (do 1.34) podawal godzine 
automatycznego rozpoczecia kasowania. Pora kasowania zawarta jest w 
tabeli CRONTAB.BCM.

\\PWD=HASLO

\\PWGEN

Syntaktyka: PWGEN <nazwa zbioru>

Sluzy do wygenerowania hasla dostepu o dlugosci 1620 przypadkowych znakow.
Haslo to jest wykorzystywane przez rozkaz PRIV np. w celu zapoczatkowania
wymiany poczty ze skrzynkami DieBox. W trakcie wymiany poczty ze
skrzynkami BCM zalecane jest korzystanie z procedury BayCom.

\\PWLOG

Syntaktyka: PWLOG ["]<haslo>["]

wywoluje ze zbioru TRACE/PWLOG.BCM linie zawierajace podane haslo.
Hasla zawierajace odstepy musza byc podane w cudzyslowie.
Brak hasla powoduje wyswietlenie zakonczenia zbioru (zawartosci ostatnich
2 kB)
Jako haslo poszukiwania moze sluzyc data albo znak wywolawczy.
W zbiorze rejestrowane sa wydarzenia zachodzace w trakcie pracy skrzynki:
- zmiany hasel dostepu dokonane przez uzytkownikow.
- nieprawidlowo podane hasla dostepu.
- hasla dostepu podane przez operatora.
- hasla dostepu podawane w trakcie dostepu do lacza szeregowego lub
  telnet.
- nieprawidlowo podane hasla w trakcie retransmisji.
Dla kazdego z tych wydarzen rejestrowane sa czas, znak stacji i znak
wezla wejsciowego.

\\QUIT

Natychmiastowe przerwanie polaczenia ze skrzynka. Rejestrowana jest data
i czas wydarzenia. Patrz takze: HELP BYE.

\\PENSUM=QUOTA

\\QUOTA

Podaje ilosc transmitowanych przez lub do uzytkownika danych i ich
stosunek do ew. ustalonego przez operatora pensum.

\\RADIOLATARNIA=BEACON

\\RANGE=ZAKRES

\\READ

Syntaktyka: READ [<argumenty>] [<katalog>] [<zakres>]

Sluzy do odczytania podanych wiadomosci. W przypadku pominiecia nazwy
katalogu uzywany jest katalog biezacy. Dozwolone jest uzycie argumentow
wymienionych pod HELP ARGUMENTY READ. Patrz tez HELP ALTER 
i HELP ARGUMENTY.
Dopuszczalne jest uzycie malych i duzych liter.
Rozkazy:
        ALTER UREAD <argumenty> (dla skrytek)
i       ALTER IREAD <argumenty>       (dla wiadomosci ogolnych)
pozwalaja na zmiane standardowego zestawu parametrow (patrz tez: rozkaz
HELP ALTER). 
Podanie zakresu pozwala na wybor pozadanych informacji.
Przyklady:
r -h oe1kda 1-5
pozwala na odczytanie ostatnich 5 wiadomosci dla oe1kda z podaniem ich
pelnych naglowkow.
r -p- -f all 200
Odczytanie wiadomosci nr. 200 z katalogu ALL z podaniem skrzynki nadawcy.
Naglowki z trasa retransmisji sa pominiete.

\\READ.DWOJKOWE=TRANSMISJE.DWOJKOWE

\\READLOCK

Syntaktyka: READLOCK <parametr>

Ustala sposob dostepu do wiadomosci prywatnych przez rozkazy LIST/READ.

READLOCK 0 - blokada dostepu moze byc zmieniana tylko przez operatora
             skrzynki za pomoca rozkazu SETUSER <znak> READLOCK <parametr>
READLOCK 1 - blokada dostepu uruchamiana jest przez uzytkownika za
             pomoca rozkazu ALTER READLOCK <parametr>.
Spis uzytkownikow blokujacych dostep wywolywany jest za pomoca U -R.
Standardowo wiadomosci prywatne dostepne sa dla wszystkich.
Czesci dwojkowe wiadomosci moga byc odczytywane po podaniu argumentu
-Q ("quiet").
Naglowki i teksty sa opuszczane. 
Przyklad: export -b demo.lzh read -q baybox 123.

\\REGULAR_EXPRESSION 

"Regular expression - Wyrazenie skonczone" sa znakami uzytymi do
przeszukiwania celow. 
Sa uzywane w BCM plikach "convert.bcm", "reject.bcm" i najwiecej
przeszukiwania.

Okreslenie znakow 

 ^    znajduje zaczynanie lacza na poczatku lancucha znakow 
 $    znajduje koniec wiersza na koniec lancucha znakow 
 .    znajduje jakis znak 
 *    gdy lancuch znakow znajduje, ze kazdy lancuch znakow przeszukiwania
      nastapil z jakims znakiem (czy inaczej znak), 
       np.: "bo"* znajduje "bot", "bo" i "boo" etc ale nie "b"
 +    gdy lancuch znakow znajduje, ze kazdy lancuch znakow przeszukiwania
      nastapil z jakims znakiem ale nie dalej znaki, 
       np.: "bo"+ znajduje "boo" i "booo", ale nie "bo" i "be"
 \    znaczy uzyc pojscia za znakiem jako lancuch znakow przeszukiwania,
       np.: "\^" znajduje "^" i nie szukali zaczynania lacza 

 [ ]  znajduje kazdy pojedynczy znak, np. [bot] znajduje b, o albo t 
 [^]  ta negacja sposobu, np. [^ bot] znajduje wszystkie znaki ale 
      zaden b, o albo t 
 [-]  ma na mysli zakres standardowych formatow wielkosci, 
       np. [b-o] znajduje kazdy standardowy format o wielkosci posrodku b i o 

Przyklady:
 ^[WE][WU]$	znajduje jako lancuch znakow przeszukiwania "WW", "WU", "EW"
                albo "EU"
 ^ S \:		znajduje "S:" tylko na poczatku lancucha znakow przeszukiwania 

\\REJECT

Syntaktyka: REJ(ECT) [ - | <string> ]

Z mozliwoscia do wyboru "-" caly plik "reject.bcm" jest pokazany.
Jesli uzywasz <string> (regular expression) wszystkie linie zawierajace
ten <lancuch_znakow> jest pokazany. Plik "reject.bcm" definiuje, ktora
poczta jest przyjeta w BBS a ktora bedzie odrzucona - (reject) lub
zatrzymana - (hold).
Nalezy dodac, ze komentarze uzywaja ";" na poczatku wyjasnienia.
Kazde linia sklada sie z nastepujacego formatu:
<akcja> [<od] [>do] [$bid] [@at] [ .B | .P ] ;  komentarz

Pierwszy zmienna niezalezna <action> definiuje co powinno zdarzyc sie
jesli wlasciwe zestawione sa wyrazenia:
  R = msg - odrzucone
  G = msg - odrzucone, jesli przyszlo lokalnie bez AX25-PW
  E = msg - odrzucone, jesli przyszlo lokalnie bez AX25-PW/TTYPW
  F = msg - ustawione -> hold, jesli forward jest bez FORWARD-PW
  H = msg - ustawione -> hold
  L = msg - ustawione -> hold, jesli przyszlo lokalnie
  P = msg - ustawione -> hold, jesli przyszlo lokalnie bez AX25-PW
  O = msg - ustawione -> hold, jesli przyszlo lokalnie bez AX25-PW/TTYPW
Operacje E i O sa mozliwe tylko w wersji Linux/Windows!!!
  <od                     = nadawca
  >do                     = urzadzenie odbiorcze
  $bid                    = (czesc $) BID/MID
  .B                      = biuletyn
  .P                      = poczta uzytkownika
  !                       = odwrocone zestawienie
  ;                       = komentarz
  (string1,string2,...    = lub wiecej niz jeden lancuch znakow

Moze byc wiecej niz jedno wyrazenie w kazdej lini, wszystkie wyrazenia
musza zostac zestawione, by wykonac <action>. Lancuchy znakow sa uzywane
jako wyrazenia skonczone, zobacz tez HELP REGULAR.
Przyklady:
  R <CB?CB                   ; odrzuc wywolanie od CB
  R >BASAR*                  ; odrzuc poczte BASAR
  R >OPINIO* @ WW            ; odrzuc poczte dla OPINIO @ WW
  R <^DF0ARS>!^ OE3DZW $ ... ; df0ar ma zgode, by wyslac poczte oe3dzw
  R $*DBO123*                ; odrzuc jakies oferty CB bbs
  R $*NL3DGH*
  P .B                       ; zatrzymaj wszystkie biuletyny wyslane bez
                             ; ax25 hasla (holdtime musi byc ustawiony!)
  G .B                       ; Zatrzymaj wszystkie biuletyny wyslane bez
                             ; ax25 hasla
  G .B <!{^DB0,^DH8YMB       ; odrzuc wszystkie biuletyny wyslane bez ax25
                             ; hasla, tylko poczta od znaku zaczynajacego
                             ; sie od 'DB0' albo znak 'DH8YM'B zawsze jest
                             ; przyjeta

\\REMERASE

REMERASE <parametr>

Ustala sposob zdalnego kasowania wiadomosci. Zdalne rozkazy kasowania
moga byc wymieniane miedzy skrzynkami systemow BayCom, DieBox i DP.
Uzytkownik skrzynki BCM musi posluzyc sie rozkazem ERASE -F.
Parametry:
REMERASE 0 .... Zdalne kasowanie wylaczone.
REMERASE 1 .... W pelni wlaczone.
REMERASE 2 .... Rozkaz zdalnego kasowania musi pochodzic z tej samej
                skrzynki, w ktorej nadano wiadomosc.
REMERASE 3 .... Jak dla parametru 2, ale wiadomosc kasowana fizycznie
                dopiero po uplywie czasu przechowywania. Pozwala to
		operatorowi na jej odzyskanie. Wczesniejsze skasowanie
		wymaga odzyskania wiadomosci za pomoca UNERASE i
		skasowanie za pomoca ERASE.
Spis wiadomosci zdalnie skasowanych wywolywany jest za pomoca rozkazu
CHECK -R. 

\\REORG

Rozpoczyna porzadkowanie i aktualizacje danych organizacyjnych skrzynki
na podstawie jej zawartosci. Sa one zawarte w zbiorach:
BID2.BCM, BIDH2.BCM, CHECKNUM.BCM, HADRHASH.BCM, USERHASH.BCM, LIST.BCM
i CHECK.BCM.
Proces ten moze trwac nawet do kilku godzin. Nie powoduje on przerwania
pracy skrzynki. W trakcie aktualizacji spisow CHECK i BID nie mozna
nadawac biuletynow. Uzytkownik informowany jest o tym fakcie za pomoca
specjalnego meldunku.
Spisy LIST.BCM i CHECK.BCM umieszczane sa najpierw w zbiorach tymczasowych
(LIST.$$$ i CHECK.$$$), ktore otrzymuja pod koniec zwykle nazwy (LIST.BCM i
CHECK.BCM). Generowana jest takze kopia zbioru USERS.BCM.
Parametry:
 reorg b  - reorganizacja dotyczy tylko biuletynow.
 reorg c  - tylko generacja nowego spisu CHECK.
 reorg f  - tylko generacja spisow dla rozkazu LIST (poza tym jak bez 
            parametrow).
 reorg h  - dotyczy tylko adresow hierarchicznych.
 reorg i  - dotyczy tylko identyfikatorow BID.
 reorg l  - reorganizacja spisu uzytkownikow.
 reorg n  - reorganizacja spisu katalogow (CHECKNUM.BCM).
            reorg n jest zawarte w reorg c.
 reorg p  - skasowanie nieaktualnych wpisow na liscie uzytkownikow.
 reorg u  - reorganizacja katalogow prywatnych.
 reorg x  - skasowanie dubletow biuletynow.
 reorg bez parametrow oznacza: reorg b+c+h+i+p+u

reorg l, reorg p i reorg x musza byc wywolane oddzielnie. 
Proces reorg wymieniany jest w spisie PS.
Patrz tez: HELP REORG L, HELP REORG P i HELP REORG X.

\\REORG.L

Reorganizacja spisu uzytkownikow jest czasochlonna i wymaga duzo miejsca
na dysku. Powinna byc ona przeprowadzana tylko w przypadku spostrzezenia
bledow w spisie.

\\REORG.P

Kasuje ze spisu uzytkownikow (USER.BCM) wszystkich, ktorzy nie podali
skrzynki macierzytej i imienia po uplywie miesiaca. Zapobiega to
nadmiernemu rozrastaniu sie spisu mogacemu utrudnic prace REORG L.

\\REORG.X

Aktualizuje spisy identyfikatorow BID i kasuje dublety. Zmiany dokonywane 
sa najpierw w kopii spisu. W razie przypadkowego przerwania akcji nalezy
zmienic nazwy zbiorow BIDS2.REO i BIDH2.REO na odpowiednie nazwy BCM.
W trakcie reorganizacji skrzynka jest niedostepna dla uzytkownikow.
Czas trwania moze dojsc do kilku godzin.

\\REPLY

Syntaktyka: REPLY [[<katalog>] <zakres> [<tytul>]

Ulatwia odpowiedz na otrzymana wiadomosc przez wykorzystanie znakow
nadawcy i skrzynek retransmitujacych. Tytul wiadomosci uzupelniony jest
skrotem 'RE:' o ile nie jest podany w rozkazie. Rozkaz REPLY bez
parametrow odnosi sie do ostatniej przeczytanej wiadomosci. Wiadomosci,
na ktore udzielono odpowiedzi zaznaczone sa za pomoca symbolu "r" zamiast
"R" w przypadku podania parametru "K" w rozkazach DIR/LIST.
Ciag "RE:RE:RE..." powstajacy w wyniku wielokrotnych odpowiedzi
zastepowany jest przez "RE^n" gdzie n oznacza kolejny numer odpowiedzi.

\\RLOG

Syntaktyka: rlog [["]<haslo>["]]

Wyswietlenie koncowki (ostatnich 2 kB) zbioru "rejlog.bcm". Podanie hasla
powoduje wyswietlenie tylko linii, w ktorych jest ono zawarte.
Patrz tez: HELP REJECT

\\RM=RMBOARD

\\RMBOARD

Syntaktyka: RMBOARD <glowny katalog> <katalog>

Sluzy do skasowania podanego katalogu, pod warunkiem ze jest on pusty
(tzn. nie zawiera wiadomosci ani katalogow podrzednych). Aktualizowany
jest spis BULLETIN.BCM.
Przyklad: RMBOARD SOFTWARE/BAYCOM
albo RMBOARD SOFTWARE BAYCOM.

\\RMNC=FLEXNET

\\RPRG

Syntaktyka: RPRG <nazwa zbioru>

Sluzy do nadania zbioru dwojkowego. W trakcie transmisji uzywany jest
protokol Auto-Bin. Transmisja rozpoczyna sie od meldunku #BIN#, ktory
musi byc pokwitowany za pomoca #OK#. W przeciwnym przypadku transmisja
zostaje przerwana.

Patrz tez: HELP RTEXT

\\ROZDZIELNIK

W adresie biuletynu zawarty jest rozdzielnik okreslajacy kraj lub
obszar dla ktorego przeznaczony jest biuletyn.
Przyklady rozdzielnikow:
POL     Polska
DL      kraje niemieckojezyczne (DL, OE, HB)
EU      Europa
WW      caly swiat.
Dalsze przyklady: TCH, OEDL, DLOE, OKOM, OMOK, BAYCOM, AMSAT, OE, ALL, OK
                  FRA
Przyklad adresowania:  S TCPIP @ POL Spis adresow
Patrz tez: HELP ADRESOWANIE.

\\ROZKAZY

Wywoluje pelny spis rozkazow dostepnych dla uzytkownika skrzynki:
 #OK# <...>      ignorowany przez skrzynke
 AKTUELL         wywolanie aktualnosci zawartych w zbiorach AKTUELL.*.
 ALTER           sluzy do zmiany parametrow majacych wplyw na sposob
                 obslugi skrzynki.
 -CHECK    <par> ustala parametry rozkazu CHECK.
 -COMMAND  <cmd> podaje rozkazy wykonywane automatycznie na poczatku
                 polaczenia.
 -DEFAULT        przywraca standardowe wartosci parametrow.
 -DELETE         kasuje wpis na liscie uzytkownikow.
 -ECHO    0|1    wlaczenie lub wylaczenie echa rozkazow.
 -FORWARD <adres> podanie skrzynki macierzystej (odpowiada MYBBS)-
 -HELPLEVEL  <n> ustala zakres informacji w zgloszeniu skrzynki.
 -IDIR     <par> zadaje parametry rozkazu DIR dla katalogow ogolnych.
 -ILIST    <par> zadaje parametry rozkazu LIST dla katalogow ogolnych.
 -IREAD    <par> zadaje parametry rozkazu READ dla katalogow ogolnych.
 -LF         <n> podaje liczbe linii odstepu.
 -LINES      <n> podaje dlugosc strony tekstu (funkcja "more").
 -NAME    <imie> podanie imienia uzytkownika.
 -MYBBS  <adres> podanie znaku skrzynki macierzystej.
 -PROMPT <tekst> wprowadza meldunek gotowosci.
 -PW     <haslo> wprowadza haslo dostepu.
 -PWLINE     <n> transmisja dodatkowej linii przed zadaniem hasla.
 -READLOCK   <n> ustala prawa dostepu do prywatnej poczty.
 -REJECT <nazwa> wprowadza nazwy katalogow pomijanych.
 -SPEECH <prefiks> wybor jezyka obslugi.
 -UDIR     <par> zadaje parametry rozkazu DIR dla katalogow prywatnych.
 -UFWD   <trasa> podaje trase retransmisji do prywatnej skrzynki
                 uzytkownika.
 -ULIST    <par> zadaje parametry rozkazu LIST dla katalogow prywatnych.
 -UREAD    <par> zadaje parametry rozkazu READ dla katalogow prywatnych.
 BADNAMES        informacja o kryteriach selekcji przyjmowanych wiadomo-
                 sci.
 BIDLIST         wywolanie spisu identyfikatorow BID.
 BIN-TX <...>    ignorowany.
 BIN-RX <...>    ignorowany.
 BYE             przerwanie polaczenia bez aktualizacji czasu.
 CD      <nazwa> zmiana katalogu roboczego.
 CHAT  <znak> .. nadanie wiadomosci do wybranego uzytkownika (tez: TALK)
 CHECK      <..> wywolanie spisu biuletynow (+ parametry)
 COMMENT         odpowiedz na biuletyn.
 CONVERS  <znak> odpowiada rozkazowi TALK
 CP              odpowiada rozkazowi TRANSFER.
 CRONTAB         wykonywanie rozkazow w wybranych momentach czasu.
 DIR             spis tresci katalogu.
 -AFTER   <data> spis tresci poczawszy od podanej daty.
 -BOARDS    <..> spis katalogow (wszystkich lub wybranych).
 -MESSAGES  <..> spis biuletynow (wszystkich lub wybranych).
 -NEWS      <..> spis wszystkich wiadomosci, ktore naplynely od ostatniego
                 uzycia rozkazu DIR NEWS.
 -PATH           spis tras retransmisji poczty.
 -SENT    <znak> spis wiadomosci nadanych przez wymieniona stacje.
 -USERS          spis uzytkownikow, na ktorych oczekuja wiadomosci.
 --ALL      <..> spis wszystkich uzytkownikow (mozna podac jokery dla
                 ograniczenia dlugosci spisu).
 --LOCAL    <..> spis wszstkich uzytkownikow, ktorzy korzystali ze
                 skrzynki.
 --MSG      <..> spis wszystkich wiadomosci prywatnych (mozna podac haslo w
                 celu ograniczenia dlugosci spisu).
 ELOG            wywolanie spisu skasowanych wiadomosci.
 ERASE <kryterium> kasowanie wybranych wiadomosci.
 EXIT            odpowiada rozkazowi QUIT.
 FIND      <par> odpowiada rozkazowi PATH.
 FINGER          odpowiada rozkazowi User.
 FORWARD <kryterium> retransmisja wiadomosci do podanej skrzynki.
 HEADER    <par> odczytanie naglowka wiadomosci (odpowiada rozkazowi KOPF)
 HELP <rozkaz> <par> wywolanie tekstu pomocy dotyczacego wybranej wiado-
                 mosci.
                 H INDEX : wywolanie spisu hasel,
                 H ALL : odczyt calego zbioru.
 INFO            wywolanie tekstu informacyjnego.
 KOPF      <par> odczytanie naglowka wiadomosci (odpowiada rozkazowi
                 HEADER).
 LIST      <par> wywolanie spisu tresci katalogu (podobnie do DIR).
 LOG        <..> odczyt dziennika stacji (mozna podac date i znak
                 uzytkownika).
 LOGOUT          odpowiada rozkazowi QUIT.
 MAN             odpowiada rozkazowi HELP.
 MEM             wywlanie spisu zajetych obszarow pamieci.
 MSG <znak> ..   odpowiada rozkazowi TALK.
 MYBBS <adres>   wprowadzenie znaku skrzynki macierzystej (patrz A F).
 NAME <imie>     wprowadzenie imienia uzytkownika (patrz A N).
 NEXT            odczyt kolejnej wiadomosci.
 NH <adres>      wprowadzenie znaku skrzynki macierzystej (MYBBS).
 PARAMETER  <..> wywolanie parametrow konfiguracyjnych skrzynki.
 PATH <adres>    wyswietlenie trasy retransmisji do podanego celu.
 PS              wywolanie spisu aktywnych procesow.
 PURGE <katalog> kasowanie zaznaczonych wiadomosci.
 PWLOG           dziennik czynnosci zabezpieczonych za pomoca hasla.
 QUIT            zakonczenie polaczenia ze skrzynka.
 QUOTA           informacja o pensum odczytu i zapisu.
 READ <nr>       odczyt wiadomosci ze skrzynki.
 REPLY           nadanie odpowiedzi na ostatnia odczytana wiadomosc.
 SEMAPHORES      wywolanie spisu uzywanych zbiorow.
 SEND    <adres> nadanie wiadomosci.
 SLOG            wywolanie dziennika systemu.
 SFHOLD          wywolanie spisu zatrzymanych wiadomosci.
 SETLIFE <dni>   ustalenie czasu skladowania wiadomosci.
 STATUS          wywolanie danych statystycznych.
 -FORWARD        liczba wiadomosci przeznaczonych do retransmisji.
 -CPU            obciazenie jednostki centralnej.
 -LIMITS         wartosci graniczne systemu.
 -MEMORY         wykorzystanie pamieci.
 -SEMAPHORES     odpowiada rozkazowi SEMAPHORES
 SP              nadanie prywatnej wiadomosci.
 TALK <znak> ..  nadanie wiadomosci do uzytkownika skrzynki.
 TIME            odpytanie daty i czasu.
 TRANSFER <kryterium> przeniesienie wiadomosci do innego katalogu.
 UNERASE  <kryterium> odzyskanie skasowanych wiadomosci.
 UNKNOWN         wywolanie spisu wiadomosci, ktore nie moga byc
                 dostarczone.
 USERS           wywolanie spisu aktualnych uzytkownikow skrzynki.
 VERSION         wywolanie informacji o wersji programu, czasie pracy i
                 wielkosci pamieci skrzynki.
 WALL <tekst>    nadanie wiadomosci do wszystkich aktualnych uzytkownikow
                 skrzynki.
 WRITE <znak> .. odpowiada rozkazowi TALK.

\\RTEXT
(tylko sysop)
Syntaktyka: RT(EXT) <nazwa pliku>

Sluzy do odczyatnia zawartosci pliku tekstowego.

Patrz tez: HELP RPRG

\\RUNUTILS

Syntaktyka: RUNUTILS  [-l] [-s] off | <prg1> <prg2> ...

Udostepnienie uzytkownikom skrzynki wymienionych programow. Parametr OFF
uniemozliwia korzystanie z nich. Liczba programow jest dowolna, pod
warunkiem ze dlugosc linii rozkazowej nie przekracza 78 znakow.
Dalsze programy moga byc zadeklarowane w zbiorze konfiguracyjnym INIT.BCM
Dopuszczalna dlugosc linii wynosi tu 255 znakow.
W wersji dla systemu DOS wykonywanie programow przerywa prace skrzynki
natomiast w wersji dla systemu LINUS sa one wykonywane rownolegle.
Znaczenie parametrow dodatkowych:
-l - wyswietlenie dodatkowej informacji o programie.
-s - spis programow dostepnych tylko dla operatora.

\\RYAPP
(tylko sysop)
Syntaktyka: RY(APP) <nazwa_pliku>

Odczyt zbioru w protokole YAPP.
Patrz tez: HELP YAPP

\\S=SEND
\\S&F
Forward poczty i biuletynow

Patrz tez: HELP ADRES, HELP FORWARD i HELP UFWD
\\SA
Syntaktyka: SA

Rozkaz dostepny tylko dla operatora sluzy do nadania wiadomosci,
ktorej otrzymanie jest potwierdzane. Jest rzadko potrzebny.
Patrz tez: HELP SEND

\\SAVEBROKEN
(tylko sysop, init.bcm)
Syntaktyka: SAV(EBROKEN) [0 | 1 | 2 ]
(domyslnie: 0)

Ta komenda definiuje forward 7plus biuletynow i maili:
SAVEBROKEN 0   tylko poprawny 7plus plik jest przyjety
SAVEBROKEN 1   jesli uzywasz FBB forward wszystkie biuletyny 7plus sa
               przyjete, jesli uzywasz ASCII unsecure bedzie forwardowany
	       protokolem z poprawnym CRC i dlugosc akceptowany
SAVEBROKEN 2   Wszystkie biuletyny sa wykluczone.

Jesli 7plus poczta ma poprawne "(7+)" jest dodany, i zatytulowany, jesli
7plus poczta jest znieksztalcona to oznaczony jest jako "(7-)".

\\SB

Nadanie biuletynu.
Patrz tez: HELP SEND

\\SCROLLDELAY

Syntaktyka: SCROLLDELAY <parametr>

Obecnie bez znaczenia. We wczesniejszych wersjach (do 1.36)
powodowal opoznienie wyswietlania danych na ekranie.

\\STRONICOWANIE

Za pomoca rozkazu ALTER LINES ustalana jest dlugosc nadawanej strony.
Zakonczenie strony sygnalizowane jest za pomoca "+?>". Dalszy ciag
wiadomosci otrzymywany jest po nadaniu znaku RETURN. Nadanie litery C
powoduje dalsza transmisje tekstu bez podzialu na strony, litery S lub
dowolnego innego nie wymienionego tutaj znaku - przerwanie transmisji,
litery Q lub B - przerwanie polaczenia ze skrzynka. 
Parametr -C w rozkazach DIR, LIST, READ i CHECK powoduje takze transmisje
bez podzialu na strony. Transmisja ciagla (A L 0 lub argument -C) moze byc
przerwana przez nadanie znaku RETURN albo rozlaczenie sie ze skrzynka.
Patrz tez: HELP ALTER.

\\SEMAPHORES

Syntaktyka: SEMAPHORES (rozkaz ma znaczenie dla programisty lub operatora
                        skrzynki).

Sluzy do wywolania spisu otwartych zbiorow. Oprocz nazwy zbioru i liczby
dostepow podawany jest rodzaj dostepu:
Rd   read         odczyt, ktory zakonczy sie w najblizszym czasie.
LRd  long read    odczyt mogacy trwac dowolnie dlugo.
Ap   append       dopisywanie informacji na koncu zbioru.
LAp  long append  j.w. przez dowolnie dlugi czas.
Wr   write        zapis informacji.
LWr  long write   j.w. przez dowolnie dlugi czas.
Ze wzgledu na konflikty w zapisie do zbiorow niektore procesy musza byc
zawieszane lub nie moga byc uruchamiane. 

\\SEND

Syntaktyka: SEND <rubryka> [@ <adres>] [# <czas magazynowania>] <tytul>

Nadanie wiadomosci do skrzynki.
Znaczenie pol:
<rubryka> znak adresata lub nazwa rubryki. Spis rubryk wywolywany jest za
          pomoca rozkazu DIR BOARDS.
<adres>   hierarchiczny adres skrzynki docelowej, oznaczenie rejonu dla
          wiadomosci ogolnych. Mozna pominac w przypadku adresatow
	  lokalnych (korzystajacych z tej samej skrzynki). Sposob 
	  adresowania podany jest pod: HELP ADRESOWANIE.
	  Podanie nieznanego lub nieprawidlowego adresu uniemozliwia
	  retransmisje poczty. Wiadomosc pozostaje w skrzynce nadawcy.
<czas magazynowania> czas magazynowania wiadomosci w dniach (maks. 999). 
          W przypadku retransmisji dotyczy takze skrzynek docelowych.
<tytul>   powinien ulatwiac orientacje uzytkownikom skrzynki.
Przyklady:
s oe1kda list !
s baycom @ all #10 Opis wersji 1.5
s oe1kda @ oe1xab.aut.eu czesc ...

Zakonczeniem wiadomosci jest ciag NNNN, ***END, /EX lub znak CTRL-Z.
Do przerwania nadawania sluzy znak CTRL-X. Jezeli wiadomosc ma byc nadana do 
kilku adresatow mozna w celu zmniejszenia obciazenia kanalu radiowego
nadac jej tresc do siebie i nastepnie rozeslac kopie do pozadanych
adresatow poslugujac sie rozkazem TRANSFER. 
Patrz tez: HELP SEND DWOJKOWE, HELP KATALOG, HELP ADRESOWANIE, 
HELP FORWARD, HELP SEND ACK i HELP LT.

\\SEND.ACK

Umieszczenie w tresci wiadomosci ciagu "/ACK" lub "/ack" powoduje
nadeslanie pokwitowania przez skrzynke docelowa.
Wiadomosci takie zawieraja w tytule slowo "ACK:". W naglowku wiadomosci
brak jest linii R:.

\\SEND.ATTACH

Operator moze dolaczyc do wiadomosci dowolny zbior umieszczajac w tresci
wiadomosci polecenie ".attach <nazwa zbioru>". Polecenie musi byc
zapisane malymi literami i bez skrotow.
Przyklad:
s sp6bhe @ sr6dbc
Nowy program
Do wiadomosci dodaje obiecany program
.attach src/program.tgz

\\SEND.DWOJKOWE=TRANSMISJE.DWOJKOWE

\\SERV_PORT

Syntaktyka: serv_port <kanal TCP>

Przyklad: serv_port 0
(wylaczenie)
Patrz tez: HELP KANALY

\\SETLIFE

Syntaktyka: SETLIFE <katalog> <zakres> [#] <dni>

Sluzy do zmiany czasu przechowywania wybranych wiadomosci. Czas ten moze
byc zmieniony przez nadawce lub adresata. Bez podania numeru (lub numerow)
wiadomosci rozkaz dotyczy ostatnio przeczytanej.
Przyklad: SETL OE1KDA 1- 200.

\\SETUSER

Syntaktyka: SETUSER <znak> <parametr> <liczba>

Odpowiada rozkazowi ALTER, z ta roznica, ze nalezy tu podac znak
uzytkownika.
Przyklad:
  setu sp6fig f SR1BSZ
odpowiada
  call sp6fig;a f SR1BSZ
Dalsze przyklady:
  setu oe1kda name krzysztof
  setu oe1kda ttypw xyzxyz
Zakaz wstepu:
  SETUSER <znak> STATUS 2
Wylaczenie pensum (patrz HELP USERQUOTA): 
  SETUSER <znak> STATUS 1.
Normalne uprawnienia:
  SETUSER <znak> STATUS 0
Patrz takze: HELP SETUSER FHOLD i HELP ALTER SYSOP.

\\SETUSER.FHOLD

Syntaktyka: SETUSER <znak> fhold 1

Uniemozliwia retransmisje biuletynow nadanych przez wymienionego uzytkownika
skrzynki. Nie dotyczy to nadanych przez niego wiadomosci prywatnych.
Operator skrzynki moze przejrzec biuletyny przed ew. dalsza retransmisja.
Funkcja ta powinna byc uzywana tylko w wyjatkowych przypadkach.

\\SF

Syntaktyka: SF <znak> | all

Sluzy do natychmiastowego rozpoczecia retransmisji poczty do podanego
celu, ktorym moze byc skrzynka sieci lub indywidualny uzytkownik.
Rozkaz ten jest uzywany zasadniczo jedynie w czasie uruchamiania lub
sprawdzania laczy, poniewaz naplywajace wiadomosci sa retransmitowane
automatycznie.
Parametr "all" powoduje podjecie retransmisji do wszystkich partnerow
analoogicznie jak przy wywolaniu z "crontab.bcm".

\\SFHOLD

Syntaktyka: sfhold [["]haslo["]]

Spis wiadomosci zatrzymanych zanjduje sie w zbiorze "sfhold.bcm". Rozkaz
sluzy do przegladania zawartosci zbioru.

\\SHUTDOWN

Syntaktyka: SHUTDOWN [-R]

Wylaczenie skrzynki. Z konsoli mozna wylaczyc skrzynke takze za pomoca
CTRL-X. Parametr -R powoduje ponowny start komputera (tylko dla systemu 
DOS).

\\SLEEP

Syntaktyka: SLEEP <sekundy>

Rozkaz uzywany tylko w skryptach rozkazowych. Powoduje przerwanie pracy
programu na zadany czas.

\\SLOG

Syntaktyka: SLOG ["]<haslo>["]

Wywoluje koncowy fragment zbioru TRACE/SYSLOG.BCM lub linie zawierajace
podane haslo. Hasla zawierajace znaki odstepu musza byc podane w
cudzyslowiu. Jako haslo moze sluzyc data lub znak wywolawczy.
Zbior zawiera meldunki systemowe. Patrz tez: HELP TRACELEVEL.

\\SLR

Syntaktyka: SLR ["]<haslo>["]

Sluzy do wywolania danych ze zbioru TRACE/SYSLOG_R.BCM.
Haslem selekcji moze byc znak stacji lub data. Hasla zawierajace znaki
odstepu musza byc podane w cudzyslowie. Rozkaz bez hasla powoduje
wyswietlenie ostatnich 2 kB zbioru. Patrz tez: HELP TRACELEVEL.

\\SMTP

(tylko Linux/Win32)

Poczawszy od wersji 1.39n skrzynka BayCom-Box moze poslugiwac sie proto-
kolem SMTP w wymianie poczty. Protokol ten nalezy do rodziny protokolow
TCP/IP.
Uzycie protokolu SMTP pozwala na przejmowanie poczty z sieci TCP/IP i
rozpowszechnianie jej  w sieci pakiet radio.
Mozliwy jest tez odbior wlasnej poczty za pomoca protokolu POP3.
Standardowo dla protokolu SMTP przewidziany jest logiczny kanal TCP o
numerze 8025. W razie potrzeby operator moze wybrac inny dowolny numer
kanalu.
Implementacja protokolu SMTP w skrzynce BayCom-Box pozwala na rozsylanie
wiadomosci rownolegle do 100 adresatow dzieki czemu moze on byc wykorzy-
stywany do obslugi list adresowych. 
Do odbioru poczty sluzy protokol POP3, ktoremu domyslnie przypisany jest
logiczny kanal TCP o numerze 8110. Rowniez i tu operator moze wybrac inny
dowolny numer kanalu. Odczyt poczty wymaga podania w zgloszeniu znaku 
i hasla (imienia). Zbiory dwojkowe i 7Plus kodowane sa automatycznie
w oparciu o algorytm "base64". Automatycznie ustalany jest tez typ
wiadomosci "mime type".
Wiadomosci nadawane za pomoca SMTP nie ulegaja zadnym zmianom.
Patrz tez: HELP SMTP CONFIG

\\SMTP-POP3.CONFIG

Konfiguracja w Outlook Expres na przykladzie znaku sp1lop
---------------------------------------------------------
Narzedzia  -> Konta -> Dodaj -> Poczta

Nazwa wyswietlana:  (nie istotne)                -> DALEJ>
Adres email:        sp1lop@sr1bsz.sz.pol.eu      -> DALEJ>

Serwer poczty przychadzacej POP3:  44.165.25.232
Serwer poczty wychodzacej SMPT:    44.165.25.232  ->  DALEJ>

Nazwa konta:  sp1lop
Haslo:  (wpisz haslo to samo co dla http)  -> DALEJ> -> ZAKONCZ>

Teraz dalsze ustawienia Wlasciwosci konta.
------------------------------------------

Narzedzia  -> Wybrac karte: Poczta
-> podswietlic nowo utworzone konto -> WLASCIWOSCI

Wybrac karte: Ogolne
--------------------
Wpisz przyjazna nazwe: BayBox
Nazwa:          Janusz Przybylski SP1LOP
Organizacja:    Polish Amateur Radio Station
Adres e-mail:   sp1lop@sr1bsz.sz.pol.eu
Adres zwrotny:  sp1lop@sr1bsz.sz.pol.eu

- Wstawic ptaszek - Uwzgledniaj to konto przy odbiorze....

Karta: Serwery
--------------
Poczta przychodzaca(POP3): 44.165.25.232
Poczta wychodzaca(SMTP):   44.165.25.232
nazwa konta: sp1lop
Haslo:  (takie samo jak dla http)

- Wstawic ptaszek - Zapamietaj haslo

Wybrac karte: Zaawansowane
--------------------------
Poczta wychodzaca(SMTP):   8025
Poczta przychodzaca(POP3): 8110

I na koniec kliknac:  ZASTOSUJ -> OK -> ZAMKNIJ

Po uruchomieniu przegladarki nalezy jeszcze w menu "Edit/Preferences" w
"Category MailGroups/Mail Server" w lini pod POP3 zaznaczyc, ze odebrane
wiadomosci nie beda kasowane w skrzynce
                          ("Leave Messages on server after retrieval").
Oczywiscie mozna tez pozwolic na ich kasowanie.
Jest to oczywiscie przyklad i w innych przegladarkach kolejnosc i nazwy
pol moga byc inne. Jezeli skrzynka lub ktoras z innych stacji sieci nie
zawiera bazy danych adresow (DNS) zamiast adresow symbolicznych nalezy
podac adresy numeryczne IP albo wpisac nazwy i adresy systemow w zbiorze
"hosts".
Sciezki dostepu:
- w systemie Windows:    \windows\hosts
- w systemie Windows NT: \winnt\system32\drivers\etc\hosts
- w systemie Linux:      /etc/hosts
Ewentualny adres bazy DNS danych nalezy wpisac w konfiguracji sieciowej
systemu w polu "nameserver".

\\SMTP_PORT

Syntaktyka: smtp_port <kanal TCP>

Przyklad: smtp_port 8110
Patrz tez: HELP KANALY

\\SP

Nadanie wiadomosci prywatnej.
Patrz tez: HELP SEND

\\STARTFORWARD=SF

\\STARTFWD=SF

\\STATUS

Wyswietla dane statystyczne. 
Patrz takze: HELP STATUS <parametr>.
Dopuszczalnymi parametrami sa:
FORWARD     - informacje na temat retransmisji poczty.
CPU         - informacje na temat wykorzystania jednostki centralnej.
MEMORY      - informacje na temat zajetosci pamieci.
SEMAPHORES  - informacje o uzywanych zbiorach.
LIMITS      - informacje o ograniczeniach systemowych.
(Linux/Win32) IPSOCKETS  .. informacje o kanalach IP i ich wykorzystaniu.
(Linux/Win32) POPSTATE   .. informacje o adresach IP uzytkownikow POP.

\\STATUS.IPSOCKETS

Informuje o wykorzystaniu kanalow IP.

\\STATUS.POPSTATE

Informuje o dostepie uzytkownikow POP3 (ich adresach IP). Dane zawarte sa 
w zbiorze "popstate.bcm").

\\STATUS.CPU

Syntaktyka: STATUS CPU   

podaje statystyczne obciazenie jednostki centralnej w % w ciagu ostatnich 40
sekund. Wyswietlane sa nastepujace rubryki (w sumie 100%):
Running:  czas obslugi skrzynki.
Screen:   czas obslugi okienek ekranowych.
Idle:     czas jalowy lub przeznaczony do obslugi funkcji wewnetrznych.

\\STATUS.FORWARD

Syntaktyka: STATUS FORWARD  ST F  - dane na temat retransmisji poczty.

Zawarte sa w nich nastepujace informacje:
- Znak sasiedniej skrzynki
- Ok: 0 - skrzynka byla nieosiagalna ostatnim razem
      1 - skrzynka byla osiagalna
      2 - odbior poczty
      3 - nawiazywanie polaczenia
      4 - transmisja poczty
- Login: uplyw czasu od ostatniego polaczenia nawiazanego przez sasiada
- ConOk: uplyw czasu od ostatniego owocnego polaczenia
- RTT:   czas niezbedny na nawiazanie polaczenia
- User:  liczba prywatnych wiadomosci przeznaczonych do retransmisji
- Info:  liczba ogolnych wiadomosci przeznaczonych do retransmisji
- E/M:   liczba wiadomosci przeznaczonych do skasowania po retransmisji

\\STATUS.LIMITS

Wyswietla informacje odnosnie obciazenia systemu i wartosci granicznych
niektorych parametrow programu. Wartosci te roznia sie dla wersji DOS,
LINUX i BCM32.

\\STATUS.MEMORY=MEM

\\STATUS.SEMAPHORES=SEMAPHORES

\\STIMEOFFSET

(Tylko w wersji DOS)
Syntaktyka: stimeoffset <liczba godzin>

Podaje roznice pomiedzy czasem zimowym danej strefy i czasem UTC.
Przelaczenie na czas letni nastepuje automatycznie (w konfiguracji b).
Mozliwe sa dwie zasadnicze konfiguracje:
a) zegar komputera nastawiony jest na czas UTC: 
   pcisutc 1, stimeoffset -1
b) zegar komputera nastawiony jest na czas lokalny: 
   pcisutc 0, stimeoffset -1
Patrz tez: HELP PCISUTC.

\\SYSOPBELL

Syntaktyka: SYSOPBELL 0 | 1

Wystepuje tylko w wersji dla systemu DOS. Powoduje wlaczenie (1)
lub wylaczenie dzwiekowej sygnalizacji polaczen ze skrzynka i ich
zakonczenia.

\\SYSOPCALL

Syntaktyka: SYSOPCALL <znak>

Podaje znak, pod ktorym operator jest automatycznie zameldowany po
podaniu jako rozkazu kropki na konsoli skrzynki. Do odpytania znaku
sluzy rozkaz PAR SYSOP.

\\TAIL

Syntaktyka: TAIL [-F] <nazwa zbioru>

Sluzy do wyswietlenia zakonczenia (ostatnich 2 kB) podanego zbioru.
Parametr -F pozwala na wyswietlanie dopisywanych w tam czasie informacji.
Wymaga on uprzedniego zaladowania sterownika SHARE.EXE.

\\TALK

Syntaktyka: TALK <znak> <tekst>

Sluzy do nadania pojedynczej linii tekstu do podanej stacji, pod
warunkiem ze jest ona aktualnie polaczona ze skrzynka.

T <znak>
Nastepujace teksty, az do podania rozkazu /q sa nadawane do podanej
stacji.

T ALL <tekst>
Sluzy do nadania wiadomosci do wszystkich polaczonych stacji (odpowiada
WALL).

Zamiast rozkazu TALK mozna uzyc jednego z nastepujacych: MSG, WRITE lub
CONVERS w zaleznosci od upodoban albo przyzwyczajenia. Nalezy pamietac,
ze bufor odbiorczy ma dlugosc jednej linii i unikac nadawania zbyt
dlugich wiadomosci.
Przyklad:
t oe1kda Czesc Krzysiek

\\TELL
Syntaktyka: TELL <bbs> <komenda>

Zdalne wykonania komend na innym Mailboxie typu BCM.
Przyklad: TELL TU5EX d b   - powoduje wykonania komendy: dir board
                             na Mailboxie TU5EX i odeslanie mailem
			     zawartosci tej komendy.
Komendy na zdalnym komputerze beda wykonane o ile sysop tamtejszy uaktywni
komende TELMODE.

Patrz tez: HELP TELLMODE

\\TELLMODE 

(tylko sysop, init.bcm)
Syntaktyka: TELLM(ODA) [ 0 | 1 | 2 ]
(wartosc domyslna: 2)

TELLMODE 0: BBS nie wykonuje komendy TELL
TELLMODE 1: tell komenda jest dozwolona ale nie wykonywalna
TELLMODE 2: wykonanie tell - komenda jest dozwolona

\\TELNET

telnet localhost bcm

Wywolanie skrzynki w protokole telnet w laczach radiowych jest
nieefektywne ze wzgledu na ich obciazenie jednak mozna z niego korzystac
w sieciach kablowych.
Dla protokolu telnet przypisany jest domyslnie kanal 4719. 
Operator moze dokonac zmiany kanalu poslugujac sie rozkazem:
 "bcm -t <numer>".
Podanie rozkazu bez numeru kanalu powoduje wylaczenie dostepu telnet.
O tym czy uzytkownik musi podawac haslo dostepu decyduje zawartosc zbioru
"rhosts.bcm". Stacje wpisane tam nie musza podawac hasla - sa uznane za
godne zaufania i otrzymuja uprawnienia operatora. Moga one korzystac z
rozkazu "connect".
Wszyscy inni uzytkownicy musza podac haslo ustalone przez operatora
(haslo TTY przy dostepie przez siec kablowa).
Po prawidlowym podaniu hasla nie otrzymuja oni jednak uprawnien operatora.

\\TELNET_PORT

Syntaktyka: telnet_port <kanal>

Przyklad: telnet_port 4719
Patrz tez: HELP KANALY

\\TGREP

Syntaktyka: TGREP ["]<haslo>["] <znak skrzynki>

Pozwala na selekcje ze zbioru TRACE/T_<znak skrzynki>.BCM pozadanych
informacji. Zbior ten zawiera dziennik retransmisji ze skrzynka
partnerska. 
Haslo zawierajace znaki odstepu musi byc podane w cudzyslowie.

\\TIME

Podaje aktualny czas.

\\TIMESLOT

Syntaktyka: TIMESLOT <czas>

Rozkaz bez znaczenia. W poprzednich wersjach (do 1.36) sluzyl do
rezerwacji nieprzerwanego czasu pracy dla poszczegolnach procesow.

\\TNC

Syntaktyka: TNC <rozkaz pcflex>

Wystepuje tylko w wersji dla systemu DOS.
Sluzy do zmiany parametrow wezla PC/Flex. Nie wszystkie rozkazy wezla
funkcjonuja w ten sposob zgodnie z oczekiwaniami. Korzystniejszym
rozwiazaniem jest posluzenie sie rozkazami FSET przed uruchomieniem
programu skrzynki.
Patrz tez: HELP TNC TRACE.

\\TNC.TRACE

Wystepuje tylko w wersji dla systemu DOS.
tnc trace -1       dotyczy wszystkich kanalow.
tnc trace <kanal>  dotyczy wybranego kanalu.
tnc trace          wylaczenie podgladu.
Podgladanie wezla za pomoca TRACE mozliwe jest tylko po wylaczeniu
funkcji monitora skrzynki.

\\TRACELEVEL

Syntaktyka: TRACELEVEL <parametr>

Pozwala na wybor meldunkow rejestrowanych w zbiorze SYSLOG.BCM:
0..#R..REPORT... wszystkie meldunki rejestrowane w zbiorze:
                 trace/syslog_r.bcm
                 Nie zalecane ze wzgledu na duza ilosc danych.
                 Meldunki systemowe (#L,#S,#F,#A) rejestrowane sa w
                 trace/syslog.bcm.
1..#L..LOG...... rejestracja tylko meldunkow systemowych.
2..#S..SERIOUS.. rejestracja tylko meldunkow bledow.
3..#F..FATAL.... tylko meldunki o powaznych bledach.
4..#A..ABORT.... tylko meldunki o bledach powodujacych przerwanie pracy
                 programu.
5......OFF...... brak rejestracji.
Do przegladania spisow sluza rozkazy SLR i SLOG. 
Patrz: HELP SLR i HELP SLOG.

\\TRANSFER

Syntaktyka: TRANSFER <katalog> <zakres> [>] <katalog_doc.> [@ <skrz.>][# <czas>][tyt.]

Powoduje przesuniecie wiadomosci do innego katalogu lub nadanie jej do
podanej skrzynki. Parametry rozkazu odpowiadaja parametrom rozkazu SEND.
Rozkaz moze byc uzywany jedynie przez adresata lub nadawce wiadomosci.
Oryginaly wiadomosci ogolnych sa kasowane, oryginaly prywatnych - nie.
Znak > moze byc opuszczony. Nazwa katalogu docelowego musi byc rozna od
nazwy katalogu zrodlowego. Rozkaz bez numeru wiadomosci odnosi sie do
ostatnio przeczytanej wiadomosci. Rozkaz nie moze byc stosowany do
zbiorow dwojkowych. 
Patrz takze rozkazy: FORWARD i LIFETIME.
Przyklad:
tr baycom 5-6 software

\\TRANSMISJE.DWOJKOWE

1. Przebieg transmisji zbiorow dwojkowych do skrzynki BCM:
   - nalezy nawiazac polaczenie ze skrzynka
   - nadac rozkaz SEND i podac tytul wiadomosci.
   - na poczatku wiadomosci dwojkowej mozna zamiescic krotki tekst
     informacyjny (nie jest to obowiazkowe).
   - nastepnie nalezy w programie terminalowym wywolac protokol AUTO-BIN
     i nadac zbior.
   - zakonczenie transmisji nastepuje automatycznie po nadaniu calosci
     zbioru, zbedne jest nadawanie znakow CTRL-Z lub ciagow ***END jak
     to ma miejsce w przypadku transmisji tekstowych.
   - w odpowiedzi skrzynka BCM nadaje obliczona sume kontrolna.
2. Odczyt zbiorow dwojkowych ze skrzynki BCM:
W spisie tresci (DIR) tytuly zbiorow dwojkowych poprzedzone sa symbolem
BIN. W celu wczytania zbioru nalezy przed podaniem rozkazu READ dla
skrzynki wlaczyc protokol AUTO-BIN w programie terminalowym.
Odbierany zbior jest automatycznie rejestrowany w katalogu podanym w
konfiguracji programu terminalowego.


\\TTYMODE

Syntaktyka: TTYMODE off|<nr-zlacza>:<szybk.>,<parzystosc>,<bity-danych>,<bity-stop>,E|L

Sluzy do zdefiniowania parametrow zlacza szeregowego, np.
  TTYMODE 1:9600,n,8,1,e
Do wylaczenia zlacza sluzy TTYMODE OFF (stan domyslny).

\\UFWD

Uzytkownicy skrzynki BCM moga skorzystac z automatycznego dostarczania
poczty do skrzynki prywatnej. Mozliwa jest takze automatyczna wysylka
biuletynow i wiadomosci prywatnych. Wszystko to wymaga spelnienia
nastepujacych warunkow:
- zadeklarowania jako skrzynki macierzystej skrzynki dostarczajacej
  poczte, a nie prywatnej.
- wprowadzenia trasy polaczenia za pomoca rozkazu:
  ALTER UFWD <znak> [<wezel>]
Skrzynka BCM podejmuje proby dostarczenia poczty za kazdym razem po
otrzymaniu nowej wiadomosci. Proby nie sa ponawiane w przypadku gdy
skrzynka prywatna jest nieosiagalna za pierwszym razem, a jedynie po
nadejsciu nastepnej wiadomosci.
Skrzynka prywatna moze zazadac dostarczenia poczty przez nadanie na
poczatku polaczenia ciagu "F>" ew. poprzedzonego identyfikatorem
systemu (SID).
Identyfikator systemu jest ciagem znakow zawartych w nawiasach
kwadratowych.
Jego zadaniem jest poinformowanie skrzynki partnerskiej o stosowanym
trybie retransmisji.
Niektore z programow (np. TCPIP) nadaja identyfikator automatycznie na
poczatku polaczenia. Do celow retransmisji uzywany jest oddzielny
identyfikator wtorny (SSID) skrzynki BCM.
Patrz takze: HELP ALTER UFWD i HELP UFWD HASLO

\\UFWD.PASSWORD=UFWD.HASLO 

\\UFWD.HASLO

Uzytkownicy korzystajacy z hasla dostepu do skrzynki musza podawac je
takze w trakcie automatycznej wymiany poczty. Sposob podania hasla jest
identyczny jak w przypadku zwyklego dostepu do skrzynki. Ciag liczb
losowych nadawany jest po identyfikatorze systemu skrzynki BCM, skrzynka
prywatna podaje haslo takze po nadaniu identyfikatora systemu.
Przyklad:
[BayCom-1.38-DH$] 21 34 22 12 3
>
[BayCom-1.38-DH$] sajkdhfeufhndjffjdshfsdf
>
Alternatywnie mozna podac haslo w trybie stosowanym przez skrzynki
systemu DieBox.

\\UFWD.PW=UFWD.HASLO

\\UIMPORT

Dokonuje konwersji danych (spisow uzytkownikow, parametrow) z formatu
DieBox (w wersji 1.9xxx) na format uzywany przez BCM. Dane te zanajduja
sie w zbiorach USER3.IDX i USER.DAT. 
Do konwersji wiadomosci mozna posluzyc sie programem BOX2BCM autorstwa
OE6BUD.

\\UNERASE

Syntaktyka: UNERASE <nazwa katalogu> <zakres>

Sluzy do odzyskania skasowanych wiadomosci. Ostateczne kasowanie
wiadomosci za pomoca rozkazu PURGE dokonywane jest automatycznie raz
na dobe (przewaznie w godzinach nocnych).
Rozkazy LIST -V i DIR -V uwzgledniaja w spisie tresci takze wiadomosci
skasowane za pomoca rozkazu ERASE. Po numerze wiadomosci podana jest
dodatkowa informacja w postaci literowej:
 E  - wiadomosc skasowana przez nadawce lub adresata.
 K  - wiadomosc skasowana zdalnie przez operatora skrzynki.
 F  - wiadomosc skasowana po retransmisji.
 L  - skrzynka lezala na trasie retransmisji, wiadomosc byla skladowana
      tylko na czas retransmisji.
 S  - wiadomosc skasowana lokalnie przez operatora.
 T  - wiadomosc zostala przeniesiona do innego katalogu.

Przyklad: 
un oe1kda 1-   odzyskuje wszystkie skasowane wiadomosci

\\UNKNOWN

Syntaktyka: UNKNOWN ["]<haslo>["]

Sluzy do wywolania ze zbioru TRACE/UNKNOWN.BCM linii zawierajacych podane
haslo (np. date lub znak wywolawczy). 
Hasla zawierajace znaki odstepu musza byc podane w cudzyslowie.
Zbior zawiera spis niedostarczonej poczty. Zbior moze byc takze
odczytywany przez programy dodatkowe jak UNK.EXE. W przypadku braku
hasla odczytywane jest zakonczenie zbioru o dlugosci 2 kB.

\\USER.ZNAK

Syntaktyka: U <znak>

Wywoluje liste parametrow ustalonych dla podanego uzytkownika.

U <argument> <haslo> pozwala na przeszukiwanie banku danych o uzytkowni-
kach. Mozliwe jest podanie nastepujacych argumentow:
-a   wyswietlenie wszystkich wpisow.
-c   podanie jedynie liczby znalezionych wpisow.
-d   ustawiony parametr FHOLD.
-f   wyswietlenie tylko pierwszego znalezionego wpisu.
-h   uzytkownik korzysta z pomocy na poziomie zerowym.
-l   uzytkownik korzystal przynajmniej raz ze skrzynki.
-m   uzytkownik sam podal znak swojej skrzynki docelowej.
-n   znak skrzynki docelowej znany, ale nie podany przez uzytkownika.
-o   uzytkownik nie korzystal nigdy ze skrzynki.
-p   uzytkownik zablokowal dostep za pomoca hasla.
-q   znaki stacji, ktore posluzyly sie rozkazem "newcall"
-r   wiadomosci prywatne niedostepne dla innych.
-s   poziom uprawnien uzytkownika jest wyzszy od zera.
-t   uzytkownik zablokowal dostep lokalny za pomoca hasla.
-u   nieznany znak skrzynki docelowej uzytkownika.
-w   uzytkownik korzysta z automatycznej wymiany poczty.
-y   kasowanie zablokowane za pomoca NOPURGE.

Hasla dla przeszukiwania:
  ... znak      c=... znak   n=... imie   l=... data polaczenia
@=... skrzynka docelowa v=... via    s=... jezyk

Wyswietlane sa wpisy, ktore spelniaja wszystkie zadane kryteria
jednoczesnie (w przypadku podania ich wiekszej liczby).
Kryterium l=... oznacza poczatkowa date, a nie wybrany dzien.

Przyklad:
u -lh @=bbs
wywoluje wpisy wszystkich stacji, ktore korzystaly przynajmniej raz ze
skrzynki, korzystaja z pomocy na poziomie 0 i dla ktorych znak skrzynki
docelowej zawiera ciag liter: bbs.
Inne przyklady:  u kda       u -c sp        u -l n=adam

Format spisu:
Znak   Mybbs               Imie         Logs Last via      PTHLRS Jezyk
OE1KDA @OE1XAB.#OE1.AUT.EU Krzysztof    1250  26h OE1XAB   000100 SP

Niektore z pol wymagaja dodatkowych wyjasnien:
Logs: sumaryczna liczba polaczen ze skrzynka.
Last: odstep czasu od ostatniego polaczenia.
via:  znak stacji wejsciowej.
U:    1: uzytkownik korzysta z automatycznej wymiany poczty.
N:    1: kasowanie zablokowane (NOPURGE).
P:    0: brak hasla,
      1: podane haslo dostepu,
      2: wylaczone zabezpieczenie za pomoca hasla dostepu.
T:    0: brak hasla,
      1: podane haslo dostepu lokalnego,
H:    poziom tekstow pomocniczych.
L:    liczba linii odstepu.
R:    0: wiadomosci prywatne dostepne dla wszystkich, 
      1: tylko wiadomosci przeczytane, 
      2: niedostepne.
S:    0: zwykli uzytkownicy, 
      1: brak ograniczen ilosci danych, 
      2: wylaczeni z korzystania ze skrzynki.
Jezyk: np. PL, DL, GB.
Imie odczytywane jest ze spisu w skrzynce albo z naglowka wiadomosci.

\\USERFWD=UFWD

\\USERLIFE

Syntaktyka: USERLIFE <dni>

Sluzy do zadania czasu skladowania wiadomosci prywatnych. Uzytkownicy
moga przedluzyc czas skladowania do podwojnej podanej tutaj wartosci.
Zamiast wartosci 0 wstawiany jest czas 999 dni, co oznacza nieograniczony
czas skladowania. 

\\USERPATH

Syntaktyka: USERPATH <sciezka dostepu>

Odpowiada rozkazowi INFOPATH dla wiadomosci prywatnych. Pod systemem DOS
zalecane jest umieszczenie wiadomosci na odrebnym dysku logicznym.

\\USERPW

Syntaktyka: USERPW <parametr>

USERPW 0 - uzytkownicy nie moga definiowac wlasnych hasel dostepu.
           Poprzednio uzywane hasla sa uzywane w dalszym ciagu.
USERPW 1 - definiowanie dozwolone. 
Operator skrzynki moze zawsze wprowadzac hasla dostepu.
Rozkaz A PW OFF sluzy do wylaczenia zabezpieczenia przez haslo.

\\USERQUOTA

Syntaktyka: USERQUOTA <kbajty>

Ustala maksymalne dzienne pensum odczytu danych. Po jego przekroczeniu
uzytkownik moze odczytywac i nadawac jedynie poczte prywatna.
Wartosc zero powoduje zniesienie ograniczenia. Ograniczenie pensum
funkcjonuje prawidlowo tylko dla MAXLOGINS 1. Rozkaz QUOTA pozwala na
zorientowanie sie w stopniu wykorzystania pensum.

\\USERS

Syntaktyka: USERS [*]

Wywoluje liste stacji polaczonych ze skrzynka.
W liscie zawarte sa informacje o akcjach podejmowanych przez uzytkownikow.
Podanie gwiazdki jako parametru rozkazu powoduje umieszczenie w spisie
znakow stacji z ktorymi uzytkownicy sa polaczeni. 

Idle     uzytkownik nie podejmuje zadnej akcji (czas jalowy).
         Mozliwosc wywolania go za pomoca rozkazu TALK.
Read     odczytywanie wiadomosci ze skrzynki.
Send     transmisja wiadomosci do skrzynki.
Search   przeszukiwanie katalogow (rozkazy DIR, LIST albo CHECK).
FwdTX    retransmisja poczty do dalszych skrzynek.
FwdRX    odbior poczty.
You      akcja wlasna.
Patrz tez: HELP USER ZNAK i HELP PS.

\\USERSF=UFWD

\\USVSENSE

Syntaktyka: USVSENSE 0 | 1

Wystepuje tylko w wersji dla systemu DOS.
Rozkaz pozwala na skorzystanie z sygnalu alarmowego doprowadzonego
z zasilacza bezprzerwaniowego do nozki 15 zlacza LPT1.
Poziom 5V oznacza prace normalna, 0 - alarm.
Parametr 1 oznacza wlaczenie, 0 - wylaczenie funkcji.

\\UTCOFFSET

Syntaktyka: UTCOFFSET <godziny> 

Definiuje roznice czasu w stosunku do czasu UTC.
UTCOFFSET -1  -  czas zimowy.
UTCOFFSET -2  -  czas letni.

\\UWIN

Wystepuje tylko w wersji dla systemu DOS i powoduje otwarcie okna
zawierajacego spis aktualnych uzytkownikow. Do powiekszenia okna mozna
posluzyc sie rozkazem ALT-R lub mysza.
Znaczenie liczb wyswietlanych obok znakow:
  0:brak akcji     1:lacze retransm.    2:retr. nad.   3:retr. odb.
  4:nad.           5:odczyt             6:int. rozk.   7:poszukiwanie
  8:kasow.         9:reorg.

\\UZYTKOWNICY=USERS

\\VERSION

Podaje numer wersji oprogramowania oraz wolne obszary pamieci RAM i na
twardym dysku. Podanie parametru 'runtime' powoduje podanie czasu
nieprzerwanej pracy komputera. Parametr 'CPUindex' powoduje podanie
wzglednej, orientacyjnej szybkosci pracy jednostki centralnej.
Przyklad:
Date: 13.12.2004 19:06:47l

OpenBCM V1.06PL (Linux)
(c) GNU GPL 1992-2005 F. Radlherr, DL8MBT et al.
OpenBCM is maintained by Markus Baumann, DH8YMB
      http://dnx274.dyndns.org/baybox
Polskie komunikaty systemowych Janusz, SP1LOP
Compiled Mar 07 2005 20:00:37 with support for:
filesurf mailserver pocsagserver servif radioif
mdpw serial guest ax25k autofwd runutils
macro yapp filefwd telnetfwd didadit userLT
fwd-edit rej-edit conv-edit extract fbbcheckmode
users4convert fts oldmailimport linuxsystemuser
Uptime: 03:37
CPU: 6(GenuineIntel) 552109238300469 ticks BogoMips: 799.53
OS: Linux version 2.4.21
PR-Interface: OpenBCM-L2 200 ports, Linux Kernel AX25
Available Memory / Used Swap                : 102676 kB / 82776 kB
Info-Path: /home/bcm/info                   : 2292 MB  (5540 MB)
User-Path: /home/bcm/user                   : 2292 MB  (5540 MB)
BCM-Home:  /bcm                             : 2292 MB  (5540 MB)
Filesurf:  /bcm/srv                         : 2292 MB  (5540 MB)

\\W2

Dostepny tylko z konsoli w wersji dla systemu DOS. Otwiera nowa sesje
polaczenia ze skrzynka. Sesja korzysta z wlasnego okna. Do zmiany okna
roboczego sluzy kombinacja ALT-<nr. okna>. Okno robocze wyroznia sie
kolorem. Do otwarcia sesji sluzy takze kombinacja CTRL-F3.

\\WALL

Syntaktyka: WALL <tekst>

Sluzy do nadania tekstu do wszystkich uzytkownikow skrzynki.
Patrz takze HELP TALK.

\\WATCHDOG

Syntaktyka: WATCHDOG <parametr>

0 kontroler pracy programu wylaczony.
1 kontrola ukladowa, na wyjsciu D0 (n. 2) zlacza LPT1 generowana jest
  fala prostokatna o czestotliwosci 10 Hz. Tylko w wesji dla systemu DOS. 
2 kontrola programowa w odstepach co 1 minute, po przejsciu na poziom 
  DOS - co 30 min. Po przekroczeniu czasu nastepuje ponowny start
  komputera.
4 Rejestracja stanu programu w dziennniku SYSLOG.BCM (wpisy oznaczone #A)
  przed uplywem czasu.
Parametr moze byc suma poszczegolnach skladnikow, np. 7 oznacza wybor
wszystkich mozliwosci. W praktyce wystarczy wartosc parametru rowna 2.

\\WBIN=WPRG

\\WPRG

Syntaktyka: WPRG <zapis zbioru>

Zapis zbioru dwojkowego przy wykorzystaniu protokolu #BIN#.
Nazwa zbioru w naglowku jest ignorowana.

\\WRITE=TALK

\\WTEXT

Syntaktyka: WTEXT <nazwa zbioru>

Zapis zbioru tekstowego, zakonczeniem jest CTRL-Z.
Przyklad:
WT MSG/CTEXT.SP.

\\WWW=HTTP

\\WHOAMI 

Syntaktyka: WH(OAMI)

Pokazuje posiadane imie i znak. 
To jest taka sama komenda jak Linux "whoami".

\\WX 

(tylko sysop, tylko Linux, init.bcm, _WXSTN opcje w config.h)
Jesli modul WXSTN jest zestawiony do wersji linux OpenBCM,
stacja wx typu "Peetbros Ultimeter 2000" (widzi http://www.peetbros.com)
moze zostac uzyty bezposrednio z OpenBCM.

Modul WX zostal rozwiniety w 1998 przez OE3DZW. Nie moze byc uzyty z 
DOS lub Windows, tylko z wersja Linux!

Gdy uruchamiajac OpenBCM z modulem WX sysop ma nastepne nowe
komendy:
- wxtty 
- wxstnname 
- wxpath 
- wxsensoraltitude 
- wxqthaltitude 

Zobacz tez HELP WXPATH, HELP WXQTHALTITUDE, HELP WXSENSORALTITUDE,
HELP WXSTNNAME" i HELP WXTTY"

\\WXPATH 

(tylko sysop, tylko Linux, init.bcm, _WXSTN opcje w config.h)
Syntaktyka: WXPATH <path> 

Definiuje sciezke dostepu dla danych wx, e.g. "wxpath/bcm/wxdata".

Zobacz tez pomoc WX 

\\WXQTHALTITUDE 

(tylko sysop, tylko Linux, init.bcm, _WXSTN opcje w config.h)
Syntaktyka: WXQTHALTITUDE <n> 

Definiuje wysokosc <n> w metrach stacji WX.

Zobacz tez pomoc WX 

\\WXSENSORALTITUDE 

(tylko sysop, tylko Linux, init.bcm, _WXSTN opcje w config.h)
Syntaktyka: WXSENSORALTITUDE <n> 

Definiuje wysokosc <n> w metrach sensora stacji WX.

Zobacz tez pomoc WX 

\\WXSTNNAME 

(tylko sysop, tylko Linux, init.bcm, _WXSTN opcje w config.h)
Syntaktyka: WXSTNNAME <name> 

Definiuje nazwe stacji WX (maksimum. 30 symboli).

Zobacz tez pomoc WX 

\\WXTTY 

(tylko sysop, tylko Linux, init.bcm, _WXSTN opcje w config.h)
Syntaktyka: WXTTY <device> 

Definiuje port szeregowy dla stacji WX, e.g. "wxtty/dev/ttyS0".

Zobacz tez pomoc WX 

\\WYAPP

Syntaktyka: wyapp <nazwa zbioru>

Zapis zbioru przy uzyciu protokolu YAPP.
Patrz tez: HELP YAPP.

\\YAPP

YAPP jest skrotem nazwy "Yet Another Packet Protocol" i oznacza
rozpowszechniony w USA protokol transmisji zbiorow dwojkowych w sieci
pakiet radio. W Europie bardziej znany jest jego konkurent Auto-Bin.
Protokol YAPP oferuje wiecej mozliwosci anizeli Auto-Bin i dlatego tez
zostal wykorzystany do transmisji zbiorow dwojkowych przez serwer zbiorow
skrzynki BCM. Operator skrzynki moze poslugiwac sie nim rowniez i w
trakcie innych transmisji zbiorow.

Patrz tez: HELP FS YGET, HELP FS YPUT, HELP RYAPP, HELP WYAPP.

\\ZAKRES 

W rozkazach DIR, LIST, READ i ERASE mozliwe jest podanie numeru
wiadomosci lub zakresu numerow. Zakres podawany jest w postaci dwoch
liczb polaczonych kreska (przykladowo 1-7). Opuszczenie pierwszej z liczb
(przykladowo -7) oznacza wywolanie ostatnich 7 wiadomosci, opuszczenie
drugiej z nich (przykladowo 3-) - wywolanie wszystkich wiadomosci
poczawszy od 3. 
Pojedynczy numer (np. 5) oznacza tylko ta wybrana wiadomosc. Podanie w 
rozkazie nazwy katalogu powoduje przejscie do niego - w przeciwnym
przypadku wykorzystywany jest katalog wybrany uprzednio.
Nazwa aktualnego katalogu moze byc wyswietlana w meldunku gotowosci
skrzynki (patrz HELP METASYMBOLE).
Wiadomosci w katalogach numerowane sa chronologicznie, wiadomosc
najstarsza ma wiec nr. 1. Oprocz numerow jako kryterium selekcji mozna
uzyc hasla poszukiwania, a wiec slowa lub terminu wystepujacego w tytule
albo identyfikatora wiadomosci (BID). 

Przyklady: DIR OE1KDA 2-7 
(spis wiadmosci w katalogu OE1KDA od 2 do 7), 
           DIR YAESU FT411 
(spis wszystkich wiadomosci z katalogu YAESU zawierajacych w tytule 
oznaczenie FT411). 
Wynik selekcji zalezny jest w pewnym stopniu od parametrow ustawionych
za pomoca rozkazu ALTER.
