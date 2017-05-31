#include <iostream>
#include <fstream>
#include <istream>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <stdio.h>
#include "Ngram.h"
using namespace std;


int ngram_order = 3;
Vocab voc;
Ngram lm( voc, ngram_order );
int TEST_DATA_LENGTH = 50;

string articles[50][512];
vector<vector <string> > vec (50);
vector <vector <string> > ans(50);

string PhoneticList[37] = {"£t","£u","£v","£w","£x","£y","£z","£{","£|","£}","£~","£¡","£¢","££","£¤","£¥","£¦","£§","£¨","£©","£ª","£¸","£¹","£º","£¬","£­","£®","£¯","£°","£±","£²","£³","£´","£µ","£¶","£·"};

string pList = "£t£u£v£w£x£y£z£{£|£}£~£¡£¢£££¤£¥£¦£§£¨£©£ª£¸£¹£º£¬£­£®£¯£°£±£²£³£´£µ£¶£·";
// Get P(W2 | W1) -- bigram
double getBigramProb(const char *w1, const char *w2)
{
    VocabIndex wid1 = voc.getIndex(w1);
    VocabIndex wid2 = voc.getIndex(w2);
    // cout << wid1 << " " << wid2 << endl;

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);
    if(wid2 == Vocab_None)  //OOV
        wid2 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { wid1, Vocab_None };
    return lm.wordProb( wid2, context);
}

void loadSeq(char*);
void loadMap(char*);
string lines[1024];
map <string, string> mMap;


int main(int argc, char *argv[])
{  
    // load .lm file
    {
        const char lm_filename[] = "./bigram.lm";
        File lmFile( lm_filename, "r" );
        lm.read(lmFile);
        lmFile.close();
    }

    loadSeq("./testdata/1.txt");
    loadMap("./ZhuYin-Big5.map");

    ofstream  fs;
    fs.open("tt.txt",  ios::out);

    
  
    // split string to vec
    for(int i = 0; i < TEST_DATA_LENGTH; ++i){
        string s = lines[i];
        string delimiter =  "  ";
        size_t pos = 0;
        string token;
        int unit = 0;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            // articles[i][unit] = token;
            // unit ++;
            vec[i].push_back(token);
            s.erase(0, pos + delimiter.length());
        }
        
    }
    
    // viterbi throughout the sequences
     for(int i = 0; i < TEST_DATA_LENGTH; ++i){
        
        for(int j = 0; j < vec[i].size(); ++j){
            // word unit
            vector <string>  word;
           
            string s = vec[i][j];           
            string delimiter =  " ";
            size_t pos = 0;
            string token;          
            while ((pos = s.find(delimiter)) != std::string::npos) {
                token = s.substr(0, pos);             
                word.push_back(token);
                s.erase(0, pos + delimiter.length());
            }
            word.push_back(s);

            // done spliiting words        
            for(int k = 1; k < word.size(); ++k){                              
                string candidates = mMap[word[k]]; 
                if(candidates.length() == 3){
                     if(k == 1){
                         fs << word[k] << " " << getBigramProb( "", word[k].c_str()) << "#";                         
                     }
                     else{
                        fs << word[k] << " " << getBigramProb( ans[i].back().c_str(),word[k].c_str()) << "#";
                     }
                     
                     ans[i].push_back(word[k]);
                }
                   
                else{
                    vector <string>  c;
                    // split the candidats
                    string s = candidates;
                    string delimiter =  " ";
                    size_t pos = 0;
                    string token;          
                    while ((pos = s.find(delimiter)) != std::string::npos) {
                        token = s.substr(0, pos);             
                        c.push_back(token);
                        s.erase(0, pos + delimiter.length());
                    }
                    c.push_back(s);


                    double maxP = -111;
                    int flag = 0;
                    for(int t = 1; t < c.size(); ++t){ 
                        double p;
                        if(k == 1){
                            p = getBigramProb( "", c[t].c_str());                        
                        }
                        else{
                           p = getBigramProb( ans[i].back().c_str(),c[t].c_str());
                        }                        
                        if( p >= maxP && p <= -1.0001){
                            maxP = p;
                            flag = t;
                        }                       
                    }
                    fs << c[flag] << " " << maxP << "#";
                    ans[i].push_back(c[flag]);


                   
                }
               
                               
            }
            fs << endl;
            
            ans[i].push_back(" ");     
        }
        fs << endl;
        
    }

    for(int i = 0; i < TEST_DATA_LENGTH; ++i){
        for(int j = 0; j < ans[i].size(); ++j){
            fs << ans[i][j];
        }
        fs << endl;
    }



   
    



    double x = getBigramProb("¥ß","©e");
    cout << x;
  
  

    return 0;
}


void loadSeq(char* file){
    ifstream inFile(file);
    string line;
    int t = 0;
    while(getline(inFile, line)){   
      lines[t] = line;
      t++;
    }
    TEST_DATA_LENGTH = t;
    return;
}

void loadMap(char* file){  
    ifstream inFile(file);
    string line;
    int t = 0;
    while(getline(inFile, line)){   
      int pos = line.find(" ");
      string key =  line.substr(0, pos);  
      line = line.erase(0,pos);
      string value = line;
      
      mMap[key] = value;     
    }    
    return;
}

