#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "ambient/core/select.h"
#include "ambient/core/smp.h"
#include "ambient/groups/group.h"
#include "ambient/core/operation/operation.h"
#include "utils/sqlite3.c"

namespace ambient {

    void scope_select(const char* sql)
    {
        groups::group* grp;
        int i, token_len, token_t;
        char* group; 
        char* as;

        i = sqlite3GetToken((const unsigned char*)sql, &token_t);
        if(token_t == TK_ILLEGAL) return;

        i += parseout_id(sql, &group);
        i += parseout_id(&sql[i], &as);
        if(as == NULL) as = (char*)"tmp";

        scope.set_group(group);
        if(!scope.involved()) return; // to rewrite this // need to know master of profile even if I'm not in the group
        if((grp = groups::group_map(as)) == NULL){
            grp = new groups::group(as, 0, group);

            if(token_t == TK_STAR){ 
                grp->add_every(1); 
            }else if(token_t == TK_FLOAT){ 
                float part = strtof(sql, NULL);
                grp->add_every((int)(1/part)); 
            }else if(token_t == TK_INTEGER){ 
                int count = (int)strtol(sql, NULL, 10);
                grp->add_range(0, count);
            }
            grp->commit();
        }
        scope.set_group(grp);
        for(size_t i=0; i < scope.get_op()->count; i++)
            scope.get_op()->profiles[i]->preprocess(ambient::scope.get_group());
        scope.get_op()->set_scope(grp);
    }

    void scope_retain(const char* sql)
    {
    }

    int parseout_id(const char* sql, char** id)
    {
        int i = 0;
        int token_len;
        int token_t;
        do{
            token_len = sqlite3GetToken((const unsigned char*)&sql[i], &token_t);
            i += token_len;
        }while(token_t != TK_ID && token_t != TK_ILLEGAL);

        if(token_t == TK_ILLEGAL) *id = NULL;
        else{
            *id = (char*)malloc(sizeof(char)*(token_len+1));
            memcpy(*id, &sql[i-token_len], token_len*sizeof(char));
            (*id)[token_len] = 0; // end of the string
        }
        return i;
    }

}
