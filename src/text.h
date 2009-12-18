#pragma once

#include "object.h"

class Text;

typedef std::tr1::shared_ptr<Text> TextPtr;

class Text: public CObject
{
protected:
    std::string message;
    
public:
    // constructors
    Text(double x, double y, std::string m, CObject *parent=NULL);

    bool update();

    friend void render(objectPtr o);
};
