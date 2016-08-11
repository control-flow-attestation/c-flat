/* Hook trampolines */
. = ALIGN(4);
__hook_start = .;
.hook : {  *(.hook) }
__hook_end = .;

/* Hook branch table */
. = ALIGN(8);
__btbl_start = .;
.btbl : { *(.btbl) }
__btbl_end = .;

/* Loop entr/exit table */
. = ALIGN(8);
__ltbl_start = .;
.ltbl : { *(.ltbl) }
__ltbl_end = .;
