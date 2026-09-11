# V3 difficulty tuning

Difficulty.csv imports into /Game/BattleForTheA/Data/DT_Difficulty using Scripts/install_difficulty.py after the native FBattleDifficultyRow struct compiles. The Data Table is included in Mac cooking and can be edited in Unreal. Reimporting CSV replaces table values, so update the source CSV when keeping an editor tune.

All speed fields use game cm/s; RadarRange uses game cm; timers and warning times use seconds. IllegalBikeSpeed 2235.2 cm/s corresponds to 50 mph. KnifeBehavior: 0 absent, 1 lunge, 2 chase. Fraction fields use 0–1.

Implemented consumers in build 013: level-select labels/warnings, TimeLimitSeconds, Walkers + Joggers (20/50/90 total), and their spawning ratio. The lab retains 50 visitors independently of park difficulty. Timeout opens the loss/retry screen.

The remaining columns preserve the full V3 tuning requirements for upcoming park-life and combat consumers. Defining their values does not implement the corresponding NPC, enemy, crate or radar systems. Those consumers and final balance remain pending.
