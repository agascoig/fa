//
//  readvocab.hpp
//  featureattractor
//
//  Created by Alexander Gascoigne on 3/20/16.
//  Copyright © 2016 Alexander Gascoigne. All rights reserved.
//

#ifndef readvocab_hpp
#define readvocab_hpp

#include <string>
#include <vector>
#include <stdio.h>

void read_vocabulary(std::istream &is, std::vector<std::string> &vocab);
long get_token_id(const std::vector<std::string> &vocab, const std::string &token);
std::string get_string_from_id(const std::vector<std::string> &vocab, int id);

#endif /* readvocab_hpp */
