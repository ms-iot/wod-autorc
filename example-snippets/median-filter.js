var vals = "";
var vals_arr = [];
var vals_sorted = [];
var vals_filtered = "";
for(var i = 0; i < 100; i ++)
{
    val = i * (Math.floor((Math.random() * 1.5) + 0.8);;
    vals_arr[i%10] = val;
    vals_sorted = vals_arr.sort();
    vals += val + "," ;
    vals_filtered += vals_sorted[5] + ","; 
}

console.log(vals + "\n" + vals_filtered);