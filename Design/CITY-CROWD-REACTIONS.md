# City crowd contact reactions

2026-09-11 [codex-maclaptop]

Light bicycle contact (speed <= 330 cm/s) on ordinary City Sample walkers stops their movement and plays FTN_N_BlockReact_Angry once, blending in over .12 seconds and out over .25 seconds. The 2.9-second reaction raises hands and returns to a neutral pose. Both outfits use the shared compatible animation skeleton. AI resumes after the reaction, and repeat contact is suppressed during it. This replaces the whole-body sinusoidal rocking only for that light-contact case.

Full-body actions share the existing decoded animation cache. Each action bone is mapped by name through its own source skeleton, and CharacterMovement retains world/root travel ownership. The action timer runs once per body update and releases the clip after completion. Ordinary locomotion and legacy foot conversion stay in place beneath the action blend.

Severe impacts, legacy visitors and death still use existing placeholder responses. This is not realistic high-speed knockdown, ragdoll, get-up, stabbing or death acceptance. Those remain required work.

Native diagnostic: Scripts/test_mac_city_walkers.py --bump. It calls BikeImpact(220) on each outfit and checks one contact, raised hands during the clip, return to lowered hands, expired recovery timer, no whole-body roll and no significant world displacement. Four images per outfit require visual inspection. This is a response-level fixture; it does not prove collision detection from actual player riding or AI path resumption.

2026-09-11 [codex-maclaptop]: Mac build/cook passed. Both direct-response native tests exit 0: one contact, reaction expired, hand peak ~50.9 cm above actor origin, idle hands lowered, zero displacement. Inspected raised-hand and recovered-pose captures for each outfit in work/build044-city-bump-review. Legacy walking regression still passes forward toes L .5181/R .9933. Reaction-level acceptance only; desktop build remains 043 and full collision/recovery gameplay is incomplete.
