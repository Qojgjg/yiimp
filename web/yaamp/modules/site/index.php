<?php

$algo = user()->getState('yaamp-algo');

JavascriptFile("/extensions/jqplot/jquery.jqplot.js");
JavascriptFile("/extensions/jqplot/plugins/jqplot.dateAxisRenderer.js");
JavascriptFile("/extensions/jqplot/plugins/jqplot.barRenderer.js");
JavascriptFile("/extensions/jqplot/plugins/jqplot.highlighter.js");
JavascriptFile("/extensions/jqplot/plugins/jqplot.cursor.js");
JavascriptFile('/yaamp/ui/js/auto_refresh.js');

$height = '240px';

$min_payout = floatval(YAAMP_PAYMENTS_MINI);
$min_sunday = $min_payout/10;

$payout_freq = (YAAMP_PAYMENTS_FREQ / 3600)." hours";
?>

<div id='resume_update_button' style='color: #444; background-color: #ffd; border: 1px solid #eea;
	padding: 10px; margin-left: 20px; margin-right: 20px; margin-top: 15px; cursor: pointer; display: none;'
	onclick='auto_page_resume();' align=center>
	<b>Auto refresh is paused - Click to resume</b></div>

<table cellspacing=20 width=100%>
<tr><td valign=top width=50%>

<!--  -->

<div class="main-left-box">
<div class="main-left-title">domain</div>
<div class="main-left-inner">

<ul>

<li>Welcome to domain! </li>
<li>&nbsp;</li>
<li>No registration is required, we do payouts in the currency you mine. Use your wallet address as the username.</li>
<li>&nbsp;</li>
<li>Payouts are made automatically every <?= $payout_freq ?> for all balances above <b><?= $min_payout ?></b></li>
<li>&nbsp;</li>
<li>Blocks are distributed proportionally among valid submitted shares.</li>

<br/>

</ul>
</div></div>
<br/>

<!--  -->

<div class="main-left-box">
<div class="main-left-title">How to mine with domain</div>
<div class="main-left-inner">

<ul>

<table>
<thead>
<tr>
<th>Stratum</th>
<th>Coin</th>
<th>Solo</th>
<th>Wallet Address</th>
<th>RigName</th>
</tr>
</thead>
<tbody><tr>
<td>
<select id="drop-stratum" colspan="2" style="min-width: 140px">
	<option value="mine.">US Stratum</option>
	<option value="cdn.">CDN Stratum</option>
	<option value="euro.">Euro Stratum</option>
	<option value="uk.">UK Stratum</option>
</select>
</td>
<td>
<select id="drop-coin">
<option data-port='7008' data-algo='-a x17' data-symbol='BTCIL'>BitcoinIL</option>
</select>
</td>
<td>
<select id="drop-solo" colspan="2" style="min-width: 140px; border-style:solid; padding: 3px; font-family: monospace; border-radius: 5px;">
	<option value="">No</option>
	<option value=",m=solo">Yes</option>
</select>
</td>
<td>
<input id="text-wallet" type="text" size="44" placeholder="RF9D1R3Vt7CECzvb1SawieUC9cYmAY1qoj">
</td><td>
<input id="text-rig-name" type="text" size="10" placeholder="8Cards">
</td>
<td>
<input id="Generate!" type="button" value="Start Mining" onclick="generate()">
</td>
</tr>
<tr><td colspan="5"><p class="main-left-box" style="padding: 3px; background-color: #ffffee; font-family: monospace;" id="output">-a xevan -o stratum+tcp://mine.miningcoins.ca:4533 -u . -p c=SAP</p>
</td>
</tr>
</tbody></table>

<?php if (YAAMP_ALLOW_EXCHANGE): ?>
<li>&lt;WALLET_ADDRESS&gt; can be one of any currency we mine or a BTC address.</li>
<?php else: ?>
<?php endif; ?>
<li>As optional password, you can use <b>-p c=&lt;SYMBOL&gt;</b> if yiimp does not set the currency correctly on the Wallet page.</li>
<li>See the "Pool Status" area on the right for PORT numbers. Algorithms without associated coins are disabled.</li>

<br>

</ul>
</div></div><br>

<!--  -->

<div class="main-left-box">
<div class="main-left-title">Site Links</div>
<div class="main-left-inner">

<ul>

<li><b>API</b> - <a href='/site/api'>http://<?= YAAMP_SITE_URL ?>/site/api</a></li>
<li><b>Difficulty</b> - <a href='/site/diff'>http://<?= YAAMP_SITE_URL ?>/site/diff</a></li>
<?php if (YIIMP_PUBLIC_BENCHMARK): ?>
<li><b>Benchmarks</b> - <a href='/site/benchmarks'>http://<?= YAAMP_SITE_URL ?>/site/benchmarks</a></li>
<?php endif; ?>

<?php if (YAAMP_ALLOW_EXCHANGE): ?>
<li><b>Algo Switching</b> - <a href='/site/multialgo'>http://<?= YAAMP_SITE_URL ?>/site/multialgo</a></li>
<?php endif; ?>

<br>

</ul>
</div></div><br>

<div class="main-left-box">
<div class="main-left-title">domain Support</div>
<div class="main-left-inner">

<ul class="social-icons">
    <li><a href="http://www.facebook.com"><img src='/images/Facebook.png' /></a></li>
    <li><a href="http://www.twitter.com"><img src='/images/Twitter.png' /></a></li>
    <li><a href="http://www.youtube.com"><img src='/images/YouTube.png' /></a></li>
    <li><a href="http://www.github.com"><img src='/images/Github.png' /></a></li>
	<li><a href="http://www.discord.com"><img src='/images/discord.png' /></a></li>
</ul>

</div></div><br>
</td><td valign=top>
<!--  -->

<div id='pool_current_results'>
<br><br><br><br><br><br><br><br><br><br>
</div>

<div id='pool_history_results'>
<br><br><br><br><br><br><br><br><br><br>
</div>

</td></tr></table>

<br><br><br><br><br><br><br><br><br><br>
<br><br><br><br><br><br><br><br><br><br>
<br><br><br><br><br><br><br><br><br><br>
<br><br><br><br><br><br><br><br><br><br>

<script>

function page_refresh()
{
	pool_current_refresh();
	pool_history_refresh();
}

function select_algo(algo)
{
	window.location.href = '/site/algo?algo='+algo+'&r=/';
}

////////////////////////////////////////////////////

function pool_current_ready(data)
{
	$('#pool_current_results').html(data);
}

function pool_current_refresh()
{
	var url = "/site/current_results";
	$.get(url, '', pool_current_ready);
}

////////////////////////////////////////////////////

function pool_history_ready(data)
{
	$('#pool_history_results').html(data);
}

function pool_history_refresh()
{
	var url = "/site/history_results";
	$.get(url, '', pool_history_ready);
}

</script>

<script>
function getLastUpdated(){
	var stratum = document.getElementById('drop-stratum');;
	var coin = document.getElementById('drop-coin');
	var solo = document.getElementById('drop-solo');
	var rigName = document.getElementById('text-rig-name').value;
	var result = '';

	result += coin.options[coin.selectedIndex].dataset.algo + ' -o stratum+tcp://';
	result += stratum.value + 'domain:';
	result += coin.options[coin.selectedIndex].dataset.port + ' -u ';
	result += document.getElementById('text-wallet').value;
	if (rigName) result += '.' + rigName;
	result += ' -p c=';
	result += coin.options[coin.selectedIndex].dataset.symbol + solo.value;
	return result;
}
function generate(){
  	var result = getLastUpdated()
		document.getElementById('output').innerHTML = result;
}
generate();
</script>
