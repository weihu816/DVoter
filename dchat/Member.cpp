/**********************************
 * FILE NAME: Member.cpp
 *
 * DESCRIPTION:
 **********************************/

#include "Member.h"


/**
 * Copy constructor
 */
Address::Address(const Address &anotherAddress) {
    addr = anotherAddress.addr;
    port = anotherAddress.port;
}

/**
 * Assignment operator overloading
 */
Address& Address::operator =(const Address& anotherAddress) {
    addr = anotherAddress.addr;
    port = anotherAddress.port;
    return *this;
}

/**
 * Compare two Address objects
 * Return true/non-zero if they have the same ip address and port number
 * Return false/zero if they are different
 */
bool Address::operator ==(const Address& anotherAddress) {
    return addr == anotherAddress.addr && port == anotherAddress.port;
}