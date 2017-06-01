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

string articles[100][512];
vector<vector <string> > vec (50);
vector <vector <string> > ans(50);
vector <vector <string> > correctSeq(50);

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
        vec[i].push_back("<s>");
        // const char *cstr = s.c_str();
        // cout << strlen(cstr);
        // for(int j = 0; )
        string delimiter =  " ";
        size_t pos = 0;
        string token;
        int unit = 0;
        while ((pos = s.find(delimiter)) != std::string::npos) {
          
            token = s.substr(0, pos);
            // cout << token.length() << "/";
            if(token.length() == 2)
                vec[i].push_back(token);
            s.erase(0, pos + delimiter.length());
        }
         vec[i].push_back("</s>");

        
    }
    for(int i = 0; i < TEST_DATA_LENGTH; ++i){
        
        for(int j = 0; j < vec[i].size(); ++j){
            fs << vec[i][j];
        }
        fs << endl;
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
            // fs << "上一個字：" << vec[m][t-1] << endl;
             
            // fs << "目前的字:" << vec[m][t];
            // fs << endl;
            // fs << preWords.length() << preWords << endl;
            // fs << curWords.length() << curWords << endl;

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
                    // cout << getBigramProb("<s>", curW[k].c_str());
                    score[t].push_back(getBigramProb("<s>", curW[k].c_str())); 
                    path[t].push_back(0);
                }
                // fs << score[1].size();
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
                        // cout << i << " " << j << endl;
                        // fs << " Pre " <<  preW[j].c_str() << " " << " Cur " << curW[i] << " " << getBigramProb(preW[j].c_str(), curW[i].c_str());
                        // fs << " PreWord Score " << score[t-1][j] << " combine score " << p << endl;
                        if(p > maxP && p < 0){
                            maxP = p;
                            curFlag = i;
                            preFlag = j;
                        }
                        
                    }
                    // fs << preW[preFlag] << curW[curFlag] << maxP << endl;
                    score[t].push_back(maxP);
                    path[t].push_back(preFlag);
                    // fs << endl;
                }            
                

            }            
            // fs << t << " loop done" << endl;
              
   
        }
        
        int preidx = 0;
        int curidx = 0;
        // backtracking
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
        for(int i = correctSeq[m].size()-1; i >=0; --i){
            fs << correctSeq[m][i];
        }
        fs << endl;
        
    }

  



    double x = getBigramProb("明","天");
    cout << x;
    cout << getBigramProb("美","天");
    cout << getBigramProb("在","美");
    cout << getBigramProb("在","明");

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

