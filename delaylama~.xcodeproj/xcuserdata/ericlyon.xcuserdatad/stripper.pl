$x = 0;
foreach $line ( <STDIN> ){
    $sline = chomp;
    print $x, ":", $sline, "\n";
    $x++;
}
