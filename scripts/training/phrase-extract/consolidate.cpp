/***********************************************************************
  Moses - factored phrase-based language decoder
  Copyright (C) 2009 University of Edinburgh

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ***********************************************************************/

#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>

#include "tables-core.h"
#include "SafeGetline.h"
#include "InputFileStream.h"

#define LINE_MAX_LENGTH 10000

using namespace std;

bool hierarchicalFlag = false;
bool onlyDirectFlag = false;
bool phraseCountFlag = true;
bool logProbFlag = false;
char line[LINE_MAX_LENGTH];

void processFiles( char*, char*, char* );
bool getLine( istream &fileP, vector< string > &item );
vector< string > splitLine();

int main(int argc, char* argv[])
{
  cerr << "Consolidate v2.0 written by Philipp Koehn\n"
       << "consolidating direct and indirect rule tables\n";

  if (argc < 4) {
    cerr << "syntax: consolidate phrase-table.direct phrase-table.indirect phrase-table.consolidated [--Hierarchical] [--OnlyDirect]\n";
    exit(1);
  }
  char* &fileNameDirect = argv[1];
  char* &fileNameIndirect = argv[2];
  char* &fileNameConsolidated = argv[3];

  for(int i=4; i<argc; i++) {
    if (strcmp(argv[i],"--Hierarchical") == 0) {
      hierarchicalFlag = true;
      cerr << "processing hierarchical rules\n";
    } else if (strcmp(argv[i],"--OnlyDirect") == 0) {
      onlyDirectFlag = true;
      cerr << "only including direct translation scores p(e|f)\n";
    } else if (strcmp(argv[i],"--NoPhraseCount") == 0) {
      phraseCountFlag = false;
      cerr << "not including the phrase count feature\n";
    } else if (strcmp(argv[i],"--LogProb") == 0) {
      logProbFlag = true;
      cerr << "using log-probabilities\n";
    } else {
      cerr << "ERROR: unknown option " << argv[i] << endl;
      exit(1);
    }
  }

  processFiles( fileNameDirect, fileNameIndirect, fileNameConsolidated );
}

void processFiles( char* fileNameDirect, char* fileNameIndirect, char* fileNameConsolidated )
{
  // open input files
  Moses::InputFileStream fileDirect(fileNameDirect);
  Moses::InputFileStream fileIndirect(fileNameIndirect);

  if (fileDirect.fail()) {
    cerr << "ERROR: could not open phrase table file " << fileNameDirect << endl;
    exit(1);
  }
  istream &fileDirectP = fileDirect;

  if (fileIndirect.fail()) {
    cerr << "ERROR: could not open phrase table file " << fileNameIndirect << endl;
    exit(1);
  }
  istream &fileIndirectP = fileIndirect;

  // open output file: consolidated phrase table
  ofstream fileConsolidated;
  fileConsolidated.open(fileNameConsolidated);
  if (fileConsolidated.fail()) {
    cerr << "ERROR: could not open output file " << fileNameConsolidated << endl;
    exit(1);
  }

  // loop through all extracted phrase translations
  int i=0;
  while(true) {
    i++;
    if (i%100000 == 0) cerr << "." << flush;

    vector< string > itemDirect, itemIndirect;
    if (! getLine(fileIndirectP,itemIndirect) ||
        ! getLine(fileDirectP,  itemDirect  ))
      break;

    // direct: target source alignment probabilities
    // indirect: source target probabilities

    // consistency checks
    if (itemDirect[0].compare( itemIndirect[0] ) != 0) {
      cerr << "ERROR: target phrase does not match in line " << i << ": '"
           << itemDirect[0] << "' != '" << itemIndirect[0] << "'" << endl;
      exit(1);
    }

    if (itemDirect[1].compare( itemIndirect[1] ) != 0) {
      cerr << "ERROR: source phrase does not match in line " << i << ": '"
           << itemDirect[1] << "' != '" << itemIndirect[1] << "'" << endl;
      exit(1);
    }

    // output hierarchical phrase pair (with separated labels)
    fileConsolidated << itemDirect[0] << " ||| " << itemDirect[1];

    // probs
    fileConsolidated << " ||| ";
    if (!onlyDirectFlag) {
      fileConsolidated << itemIndirect[2];    // prob indirect
    }
    fileConsolidated << " " << itemDirect[2]; // prob direct
    if (phraseCountFlag) {
      fileConsolidated << " " << (logProbFlag ? 1 : 2.718); // phrase count feature
    }

    // alignment
    fileConsolidated << " ||| " << itemDirect[3];

    // counts, for debugging
    vector<string> directCounts = tokenize(itemDirect[4].c_str());
    vector<string> indirectCounts = tokenize(itemIndirect[4].c_str());
    fileConsolidated << "||| " << indirectCounts[0] << " " << directCounts[0];
    // output rule count if present in either file
    if (directCounts.size() > 1) {
      fileConsolidated << " " << directCounts[1];
    } else if (indirectCounts.size() > 1) {
      fileConsolidated << " " << indirectCounts[1];
    }

    fileConsolidated << endl;
  }
  fileDirect.Close();
  fileIndirect.Close();
  fileConsolidated.close();
}

bool getLine( istream &fileP, vector< string > &item )
{
  if (fileP.eof())
    return false;

  SAFE_GETLINE((fileP), line, LINE_MAX_LENGTH, '\n', __FILE__);
  if (fileP.eof())
    return false;

  item = splitLine();

  return true;
}

vector< string > splitLine()
{
  vector< string > item;
  bool betweenWords = true;
  int start=0;
  int i=0;
  for(; line[i] != '\0'; i++) {
    if (line[i] == ' ' &&
        line[i+1] == '|' &&
        line[i+2] == '|' &&
        line[i+3] == '|' &&
        line[i+4] == ' ') {
      if (start > i) start = i; // empty item
      item.push_back( string( line+start, i-start ) );
      start = i+5;
      i += 3;
    }
  }
  item.push_back( string( line+start, i-start ) );

  return item;
}
