#ifndef APP_DELEGATE_H
#define APP_DELEGATE_H

#include "cocos2d.h"
USING_NS_CC;

class AppDelegate : private Application
{
public:
    AppDelegate();
    virtual ~AppDelegate();

    virtual bool applicationDidFinishLaunching() override;
    virtual void applicationDidEnterBackground() override;
    virtual void applicationWillEnterForeground() override;
};

#endif
