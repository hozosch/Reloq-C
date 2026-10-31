/* Native duration-clock percentages for ECI 0..39. The source rule halves
 * speed+50 with integer truncation, evaluates exp(-.04*half) in 1/10000
 * units, then rounds 100*that/1353. The same calculation reproduces all
 * 211 existing observed duration percentages at ECI 40..250. */
static const unsigned short native_slow_percent[40]={
    272,272,261,261,251,251,241,241,232,232,
    223,223,214,214,205,205,197,197,190,190,
    182,182,175,175,168,168,162,162,155,155,
    149,149,143,143,138,138,132,132,127,127
};
