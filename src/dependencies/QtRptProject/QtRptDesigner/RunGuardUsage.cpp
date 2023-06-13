int main()
{
    RunGuard guard( "some_random_key" );
    if ( !guard.tryToRun() )
        return 0;

    QAppplication a(/*...*/);
    // ...
}