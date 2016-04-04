/**********************************
 * FILE NAME: Parser.h
 *
 * DESCRIPTION: Parser class
 **********************************/

#include "Parser.h"
#include "DNode.h"

class Parser{
private:
    DNode * node;
    
public:
    Parser(DNode * node) : node(node) {}
    
};