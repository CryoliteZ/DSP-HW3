# coding=big5
import codecs

path = 'Big5-ZhuYin.map'
path2 = 'ZhuYin-Big5.map'

d = dict()
with codecs.open(path, "r",encoding='big5', errors='ignore') as fdata:
    mylist = fdata.read().splitlines() 
    for idx, row in enumerate(mylist):
        row = row.split(' ')
        phonetic = row[1].split('/')

        # do phonetic prefix mapping
        phoneticPrefix = [ph[0] for ph in phonetic]
        phoneticPrefix = list(set(phoneticPrefix))       

        for ph in phoneticPrefix:
            if ph in d:              
                d[ph].append(row[0])
            else:
                d[ph] = [row[0]]
        # do word mapping  
        d[row[0]] = row[0]

with codecs.open(path2, "w",encoding='big5', errors='ignore') as f:    
    for key in d:
        f.write(key + ' ')
        for idx, word in enumerate(d[key]):
            if idx != (len(d[key])-1):
                f.write(word + ' ')
            else:
                f.write(word + '\n')     

# print(d['ㄇ'])