#pragma once

namespace rps
{

enum TokenType
{
    TOKEN_NOTOKEN
    , TOKEN_START_PROGRAM
    , TOKEN_END_PROGRAM
    , TOKEN_COMMAND
    , TOKEN_START_LIST 
    , TOKEN_END_LIST    //5
    , TOKEN_START_MAP
    , TOKEN_END_MAP
    , TOKEN_INTEGER
    , TOKEN_STRING   
    , TOKEN_COMMENT   // 10
    , TOKEN_IF
    , TOKEN_THEN
    , TOKEN_ELSE
    , TOKEN_ENDIF
    , TOKEN_FOR     // 15
    , TOKEN_ENDFOR
    , TOKEN_WHILE
    , TOKEN_REPEAT
    , TOKEN_ENDWHILE 
    , TOKEN_SYSTEM   // 20
    , TOKEN_EOL
    , TOKEN_EXIT
    , TOKEN_EOF
    , TOKEN_DOUBLE_QUOTED
    , TOKEN_SINGLE_QUOTED
    , TOKEN_NONE
    , TOKEN_INVALID
    , TOKEN_SHELL
};

enum ObjectType
{
    OBJECT_STRING
    ,OBJECT_INTEGER
    ,OBJECT_NONE
    ,OBJECT_LIST
    ,OBJECT_MAP
    ,OBJECT_COMMAND
    ,OBJECT_PROGRAM
    ,OBJECT_TOKEN 
    ,OBJECT_IF
    ,OBJECT_FOR
    ,OBJECT_WHILE
};

static const char *ObjectNames[] = {
    "String"
    , "Integer"
    , "None"
    , "List"
    , "Map"
    , "Command"
    , "Program"
    , "Token"
    , "If"
    , "For"
    , "While"
};

} // namespace rps

