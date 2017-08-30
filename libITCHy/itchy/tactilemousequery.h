#ifndef TACTILEMOUSEQUERY_H
#define TACTILEMOUSEQUERY_H

#include <scratchy/positionquery.h>

class TactileMouseQuery : public PositionQuery
{
public:
    TactileMouseQuery(bool detached = false, unsigned int timeout = 50);
    virtual ~TactileMouseQuery();

    // Implementations of PositionQuery interface:
    virtual QVector2D position() const;
    virtual QVector2D velocity() const;
    virtual float orientation() const;
    virtual float angularVelocity() const;
    virtual bool buttonPressed() const;

    virtual void update();
    virtual bool initialize();
    virtual void feedback(unsigned char r, unsigned char g, unsigned char b);

private:
    struct impl;
    impl* implementation;    
};

#endif // TACTILEMOUSEQUERY_H
