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

int ngram_order = 2;
Vocab voc;
Ngram lm( voc, ngram_order );
int TEST_DATA_LENGTH = 50;

string articles[100][512];
vector<vector <string> > vec (100);
vector <vector <string> > ans(100);
vector <vector <string> > correctSeq(100);

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
        // argv[6] == "bigram.lm"
        // const char lm_filename[] = ;
        File lmFile( argv[6], "r" );
        lm.read(lmFile);
        lmFile.close();
    }

    // argv[2] == "./testdata/6.txt"
    loadSeq(argv[2]);
    // argv[4] == "./ZhuYin-Big5.map"
    loadMap(argv[4]);

    // ofstream  fs;
    // fs.open(argv[9],  ios::out);    

    // split string to vec
    for(int i = 0; i < TEST_DATA_LENGTH; ++i){
        string s = lines[i];
        vec[i].push_back("<s>");       
        string delimiter =  " ";
        size_t pos = 0;
        string token;
        int unit = 0;
        while ((pos = s.find(delimiter)) != std::string::npos) {          
            token = s.substr(0, pos);          
            if(token.length() == 2)
                vec[i].push_back(token);
            s.erase(0, pos + delimiter.length());
        }
         vec[i].push_back("</s>");  
    }    

    
    // viterbi throughout the sequences
     for(int m = 0; m < TEST_DATA_LENGTH; ++m){
        vector<vector <double> > score (vec[m].size()) ;   
        vector<vector <int> > path (vec[m].size()) ;     
        for(int t = 1; t < vec[m].size(); ++t){                 
            string preWords = mMap[vec[m][t-1]];
            string curWords = mMap[vec[m][t]]; 
           
            if(t - 1 == 0) preWords = "<s>";
            if(t == vec[m].size() -1) curWords = "</s>";   
            if(curWords.length() <2){
                curWords = vec[m][t];
            }        

            // split preWords 
            vector <string> preW;
            string delimiter =  " ";
            size_t pos = 0;
            string token;          
            while ((pos = preWords.find(delimiter)) != std::string::npos) {
                token = preWords.substr(0, pos);
                if(token.length() == 2)           
                    preW.push_back(token);
                preWords.erase(0, pos + delimiter.length());
            }
            preW.push_back(preWords);   
            // split curWords 
            vector <string> curW;
            delimiter =  " ";
            pos = 0;                   
            while ((pos = curWords.find(delimiter)) != std::string::npos) {
                token = curWords.substr(0, pos);
                if(token.length() == 2)           
                    curW.push_back(token);
                curWords.erase(0, pos + delimiter.length());
            }
            curW.push_back(curWords);
            // first word ( t == 1)           
            if(t == 1){
                for(int k = 0; k < curW.size(); ++k){                   
                    score[t].push_back(getBigramProb("<s>", curW[k].c_str())); 
                    path[t].push_back(0);
                }               
            }
            else{                 
                for(int i = 0; i < curW.size(); ++i){   
                    // do biagram viterbi                    
                    int preFlag = 0;
                    int curFlag = 0;
                    double maxP = -10000;
                    for(int j = 0; j < preW.size(); ++j){
                        double p = 0.0;                        
                        p = score[t-1][j] + getBigramProb(preW[j].c_str(), curW[i].c_str());                         
                        if(p > maxP && p < 0){
                            maxP = p;
                            curFlag = i;
                            preFlag = j;
                        }
                        
                    }                   
                    score[t].push_back(maxP);
                    path[t].push_back(preFlag);                    
                }   
            }   
        }
        
        int preidx = 0;
        int curidx = 0;
        // viterbi backtracking
        for(int t = vec[m].size()-1; t >= 1; --t){
            curidx = preidx;

            string preWords = mMap[vec[m][t-1]];
            string curWords = mMap[vec[m][t]]; 

            if(t - 1 == 0) preWords = "<s>";
            if(t == vec[m].size() -1) curWords = "</s>";
            // split curWords 
            vector <string> curW;
            string delimiter =  " ";
            size_t pos = 0;
            string token;                        
            while ((pos = curWords.find(delimiter)) != std::string::npos) {
                token = curWords.substr(0, pos);
                if(token.length() == 2)           
                    curW.push_back(token);
                curWords.erase(0, pos + delimiter.length());
            }
            curW.push_back(curWords);

            // split preWords 
            vector <string> preW;
            delimiter =  " ";
            pos = 0;                   
            while ((pos = preWords.find(delimiter)) != std::string::npos) {
                token = preWords.substr(0, pos);
                if(token.length() == 2)           
                    preW.push_back(token);
                preWords.erase(0, pos + delimiter.length());
            }
            preW.push_back(preWords);

            correctSeq[m].push_back(curW[curidx]);
            preidx = path[t][curidx];
        }
        correctSeq[m].push_back("<s>");
        for(int i = correctSeq[m].size()-1; i >= 1; --i){
            cout << correctSeq[m][i] << " ";
        }
        cout << correctSeq[m][0] << endl;       
    }
    // fs.close();

    // for( int i = 0; i < 10; ++i){
    //     cout << argv[i] << " ";
    // }
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
         
      line = line.erase(0,pos+1);
      string value = line;  
      
      mMap[key] = value;     
    }

    return;
}

