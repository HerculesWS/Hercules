# Script directives

Script directives allows to give special instructions to the script parser.

## pragma
### true_duplicate
The `true_duplicate` directive dictates that the script parser should make a
true duplicate of NPCs when it encounters a `duplicate()` statement. True
duplicates do not share their local variables with their parent NPC, and their
OnInit event is always called upon duplication.

Example:
```c
#pragma true_duplicate

-	duplicate(SourceNPC)	NewNPC	FAKE_NPC
```
