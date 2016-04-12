//
//  readvocab.cpp
//  featureattractor
//
//  Created by Alexander Gascoigne on 3/20/16.
//  Copyright © 2016 Alexander Gascoigne. All rights reserved.
//

#include <iostream>
#include <vector>
#include <algorithm>
#include "readvocab.hpp"

void read_vocabulary(std::istream &is, std::vector<std::string> &vocab)
{
    std::string token;

    while (!(is >> token).eof()) {
        vocab.push_back(token);
    }
}

long get_token_id(const std::vector<std::string> &vocab, const std::string &token)
{
    std::vector<const std::string>::iterator i=lower_bound(vocab.begin(),vocab.end(),token);
    if ((i==vocab.end()) || (*i!=token))
      return vocab.size();
    return std::distance(vocab.begin(),i);
}

std::string get_string_from_id(const std::vector<std::string> &vocab, int id)
{
  if (id==vocab.size())
    return "UNKNOWN";
  return vocab[id];
}

