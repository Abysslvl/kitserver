#define UNICODE
#define THISMOD &k_afs

#include <windows.h>
#include <stdio.h>
#include <string>
#include "configs.h"
#include "utf8.h"
#include "log.h"
#include "names.h"

extern KMOD k_afs;

song_map_t::~song_map_t()
{
    for (hash_map<WORD,SONG_STRUCT>::iterator it = _songMap.begin(); it != _songMap.end(); it++)
    {
        Utf8::free(it->second.title);
        Utf8::free(it->second.author);
    }
}

song_map_t::song_map_t()
{
}

void song_map_t::update(const wstring& filename)
{
    if (GetFileAttributes(filename.c_str())==INVALID_FILE_ATTRIBUTES)
        return; // file cannot be accessed - looks like it's not there.

    // process kit map file
    hash_map<WORD,wstring> mapFile;
    if (!readMap(filename.c_str(), mapFile))
    {
        LOG(L"Unable to read songs map (%s)",filename.c_str());
        return;
    }

    for (hash_map<WORD,wstring>::iterator it = mapFile.begin(); it != mapFile.end(); it++)
    {
        // determine song name, author name
        wstring title;
        wstring author;

        wstring& line = it->second;
        int qt1 = line.find('"');
        int qt2 = line.find('"',qt1+1);
        int qt3 = line.find('"',qt2+1);
        int qt4 = line.find('"',qt3+1);

        if (qt1!=string::npos && qt2!=string::npos && qt3!=string::npos && qt4!=string::npos)
        {
            title = line.substr(qt1+1,qt2-qt1-1);
            author = line.substr(qt3+1,qt4-qt3-1);
        }
        else if (qt1!=string::npos && qt2!=string::npos)
        {
            title = line.substr(qt1+1,qt2-qt1-1);
            author = L"Unknown Artist";
        }
        else
        {
            title = line;
            author = L"Unknown Artist";
        }

        SONG_STRUCT ss;
        ss.binId = it->first;
        ss.title = (char*)Utf8::unicodeToUtf8(title.c_str());
        ss.author = (char*)Utf8::unicodeToUtf8(author.c_str());

        pair<hash_map<WORD,SONG_STRUCT>::iterator,bool> ires =
            _songMap.insert(pair<WORD,SONG_STRUCT>(it->first,ss));
        if (!ires.second)
        {
            // replace existing entry
            if (ires.first->second.title) 
                Utf8::free(ires.first->second.title);
            if (ires.first->second.author) 
                Utf8::free(ires.first->second.author);

            ires.first->second.title = ss.title;
            ires.first->second.author = ss.author;
        }

    }
    LOG(L"Songs-map (%s) read", filename.c_str());
}

ball_map_t::~ball_map_t()
{
    for (hash_map<WORD,BALL_STRUCT>::iterator it = _ballMap.begin(); it != _ballMap.end(); it++)
    {
        Utf8::free(it->second.name);
    }
}

ball_map_t::ball_map_t()
{
}

void ball_map_t::update(const wstring& filename)
{
    if (GetFileAttributes(filename.c_str())==INVALID_FILE_ATTRIBUTES)
        return; // file cannot be accessed - looks like it's not there.

    // process kit map file
    hash_map<WORD,wstring> mapFile;
    if (!readMap(filename.c_str(), mapFile))
    {
        LOG(L"Unable to read balls map (%s)",filename.c_str());
        return;
    }

    for (hash_map<WORD,wstring>::iterator it = mapFile.begin(); it != mapFile.end(); it++)
    {
        wstring& name = it->second;

        // strip off quotes, if present
        if (name[0]=='"') name.erase(0,1);
        int last = name.length()-1;
        if (name[last]=='"') name.erase(last);

        BALL_STRUCT bs;
        bs.number = it->first;
        bs.name = (char*)Utf8::unicodeToUtf8(name.c_str());

        pair<hash_map<WORD,BALL_STRUCT>::iterator,bool> ires =
            _ballMap.insert(pair<WORD,BALL_STRUCT>(it->first,bs));
        if (!ires.second)
        {
            // replace existing entry
            if (ires.first->second.name) 
                Utf8::free(ires.first->second.name);

            ires.first->second.name = bs.name;
        }
    }
    LOG(L"Balls-map (%s) read", filename.c_str());
}

