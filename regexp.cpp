// (public|private)? ?(static)? ?[[::word::]]*
static char* skip(const char* p)
{
    while(true)
    {
        switch(*p)
        {
        case '\\':
            p++;
            break;
        case 0:
            return (char*)p;
        case '(':
        case '[':
            skip(p+1);
            continue;
        case ')':
        case ']':
			return (char*)(p+1);
        }
        p++;
    }
}

static const char* next(const char* p)
{
    switch(*p)
    {
    case 0:
        return p;
    case '\\':
        return p+2;
    case '(':
    case '[':
        return skip(p+1);
    default:
        return p+1;
    }
}

static bool chain(char s, const char* e)
{
    while(true)
    {
        switch(*e)
        {
        case ']':
        case 0:
            return false;
        case '\\':
            e++;
        default:
            if(s==*e++)
                return true;
            break;
        }
    }
}

static bool group(const char* p, const char* e, const char** p2)
{
    while(true)
    {
        switch(*e)
        {
        case ')':
        case ']':
        case '|':
        case 0:
            if(p2)
                *p2 = p;
            return true;
        case '[':
            if(!chain(*p++,e+1))
                return false;
            if(p2)
                *p2 = p;
            return true;
        case '(':
            return group(p,e+1,p2);
        case '\\':
            e++;
        default:
            if(*p++!=*e++)
            {
                while(true)
                {
                    if(*e==')' || *e==0)
                        return false;
                    if(*e=='|')
                        break;
                }
                if(*e=='|')
                {
                    e++;
                    continue;
                }
                return false;
            }
            break;
        }
    }
}

bool szlike(const char* p, const char* e)
{
    while(true)
    {
        if(!e[0])
            return true;
        const char* e1 = next(e);
        switch(*e1)
        {
        case '?':
            group(p,e,&p);
            break;
        case '+':
            if(!group(p,e,&p))
                return false;
        case '*':
            while(group(p,e,&p));
            break;
        default:
            if(!group(p,e,&p))
                return false;
            break;
        }
        e = e1;
    }
}