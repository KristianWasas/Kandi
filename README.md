# Kandi
Bachelor thesis 

Työn tarkoituksena on tutkia eri konseptien implementointia minimax hakualgoritmiin shakissa. Sitten vertaillaan muodostuneita haku puita ja yritetään näin ollen parantaa alkuperäistä hakualgoritmia. 

Lukuun ottamatta paria lookuptablen osaa ja SFML kirjastoa, kaikki itse. 

Kandiin tarvittavat osat valmiit, kokonaisuus on aika ready, vielä opening book tulee jossain vaiheessa

ATM on:
- Backend, toimiva siirtogeneraattori ja evaluointi funktio, myös minimax hakualgoritmi ja tarvittavat apu funktiot bitBoardin käsittelyyn, toimii itsessään,
myös transpositiontable on implementoitu backendiin (EI NOPEASTI, HUONO IMPLEMENTOITNTI)
- Game, siellä on vain itse shakin pelaamisen tarvittavat koodit, Kandiin turha, mutta jos haluu pelata shakkia antaa mennä, AI vastustaja implementoitu myös chessAI kohdassa 
- GUI, siellä on ite SFML rajapinta ja kaikki tarvittava peli ikkunalle yms.
- Testing, Itse kandi data keruuta varten tarvittava koodi
- testData, siellä on kandissa käytettävä "raaka" data, 
- Nyt myös AI vs AI option, togglet löytyy game.cpp ja minimax.cpp kansioista, voi vertailla kahta algoritmia keskenään

Jos haluaa ladata itselleen antaa mennä, HUOM! compiling vaatii oikean g++ version ja toimii vaan Windowsilla

Oikea g++ versio on "g++.exe (Rev6, Built by MSYS2 project) 11.2.0", tarkista "g++ --version" komennolla

Voi toimia vanhemmilla/uusimmilla versiolla mutta ei takuuta

Compilaus komennolla "mingw32-make", luo .exe filen jonka voi ajaa

JOS KYSYTTÄVÄÄ kristian.wasastjerna@aalto.fi
