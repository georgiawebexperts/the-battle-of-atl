#include "../../Source/AuraPlayground/BattleSleeperTrigger.h"
#include <cassert>
int main(){
 FBattleSleeperTrigger T;
 assert(!T.Observe(500,0,true));assert(T.Observe(450,0,true));T.Attempted(false);
 for(int I=0;I<1000;++I)assert(!T.Observe(I%2?451:449,.1f,true));
 assert(!T.Observe(750,0,true));assert(!T.Observe(450,0,true));
 assert(!T.Observe(751,0,true));assert(T.Observe(450,0,true));T.Attempted(true);
 assert(!T.Observe(800,1,true));assert(!T.Observe(400,1,true));
 assert(!T.Observe(400,100,true)); // Waiting beside a sleeper does not trigger again.
 assert(!T.Observe(800,0,true));assert(T.Observe(400,0,true));
 FBattleSleeperTrigger Tutorial;
 assert(!Tutorial.Observe(400,0,false));assert(!Tutorial.Observe(400,10,true));
 assert(!Tutorial.Observe(800,0,true));assert(Tutorial.Observe(400,0,true));
 FBattleSleeperTrigger Miss;Miss.Attempted(false);
 assert(!Miss.Observe(800,29,true));assert(!Miss.Observe(400,0,true));
 assert(!Miss.Observe(800,1,true));assert(Miss.Observe(400,0,true));
}
