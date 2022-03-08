# Kandi
Bachelor thesis 

Työn tarkoituksena on tutkia eri konseptien implementointia minimax hakualgoritmiin shakissa. Sitten vertaillaan muodostuneita haku puita ja yritetään näin ollen parantaa alkuperäistä hakualgoritmia. 

Lukuun ottamatta paria lookuptablen osaa, kaikki on itse koodattu. 

Vaiheessa vielä

ATM on:
- Backend, toimiva siirtogeneraattori ja evaluointi funktio, myös minimax hakualgoritmi ja tarvittavat apu funktiot bitBoardin käsittelyyn, toimii itsessään,
myös transpositiontable on implementoitu backendiin (EI NOPEASTI, HUONO IMPLEMENTOITNTI)
- Game, siellä on vain itse shakin pelaamisen tarvittavat koodit, Kandiin turha, mutta jos haluu pelata shakkia antaa mennä, AI implementoitu myös chessAI kohdassa 
- GUI, siellä on ite SFML rajapinta ja kaikki tarvittava peli ikkunalle yms.
- Testing, Itse kandi data keruuta varten
- testData, siellä on kandissa käytettävä "raaka" data, 

Jos haluaa ladata itselleen antaa mennä, HUOM! compiling vaatii oikean version ja toimii vaan Windowsilla

Oikea g++ versio on "g++.exe (Rev6, Built by MSYS2 project) 11.2.0" 

Compilaus komennolla "mingw32-make", luo exe filen jonka voi ajaa

JOS KYSYTTÄVÄÄ kristian.wasastjerna@aalto.fi
