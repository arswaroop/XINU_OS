#include <xinu.h>
#include<kv.h>
 
 int xsh_cache_small(int nargs, char *args[]){ 
     int retval; 
     int set_errors = 0; 
     char* valtmp= NULL; 
     int get_errors=0; 
    kv_init(); 

    retval=kv_set("rregzezguuupggjr", "zazzbzyssqzuzqhwtwjmpzgjkmkpwsepnutjwitaqprggxlfrvjjwhbjkuxbgdecfziekfzrotvkhrdldwgkefetfsntigkyfahxarctwpczmllxxzyfafnfsezpysa"); 
     if(retval>0) set_errors++; 
 
    retval=kv_set("xkybdydmugxxsffw", "piwiezexsrubcxlrteoersmyyugtimnrimvvqmxlddgxugummkrwteskdmtllbyytsmqhtaudbeathwokylnltaqweykjmrialwbbneaworfweawjsqwjvwjhafsuez"); 
     if(retval>0) set_errors++; 
 
    retval=kv_set("ummaokrjqjtnjhlq", "dsymkrfpwgxkvifwgqhpatziusgebyeppofrtrcctfezxmmhchhobiwqqxdhsmhloembeucpfyptexzyblzflkoodtnreujgsrzuvtpjtoaihrfzwgqxemwbphghzaw"); 
     if(retval>0) set_errors++; 
 
    valtmp = kv_get("xkybdydmugxxsffw"); 
     if(valtmp==NULL) get_errors++; 

    valtmp = kv_get("rregzezguuupggjr"); 
     if(valtmp==NULL) get_errors++; 

    valtmp = kv_get("ummaokrjqjtnjhlq"); 
     if(valtmp==NULL) get_errors++; 

    printf("%d, %d \n",set_errors, get_errors); 
 kv_reset(); 
 } 

