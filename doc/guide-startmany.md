# start-many Setup Guide

You can also find instructions on [documentchain.org](https://documentchain.org/support/masternodes)

## Setting up your Wallet

### Create New Wallet Addresses

1. Open the Qt Wallet.
2. Click the Receive tab.
3. Fill in the form to request a payment.
    * Label: MN1
    * Amount: 5000 (optional)
    * Click *Request payment* button
5. Click the *Copy Address* button

Create a new wallet address for each Masternode.

Close your Qt Wallet.

### Send 5000 DMS to New Addresses

Send exactly 5000 DMS to each new address created above.

### Create New Masternode Private Keys

Open your Qt Wallet and go to console (from the menu select `Tools` => `Debug Console`)

Issue the following:

```masternode genkey```

*Note: A masternode private key will need to be created for each Masternode you run. You should not use the same masternode private key for multiple Masternodes.*

Close your Qt Wallet.

## <a name="masternodeconf"></a>Create masternode.conf file

Remember... this is local. Make sure your Qt is not running.

Create the `masternode.conf` file in the same directory as your `wallet.dat`.

Copy the masternode private key and correspondig collateral output transaction that holds the 5000 DMS.

*Note: The masternode priviate key is **not** the same as a wallet private key. **Never** put your wallet private key in the masternode.conf file. That is almost equivalent to putting your 5000 DMS on the remote server and defeats the purpose of a hot/cold setup.*

### Get the collateral output

Open your Qt Wallet and go to console (from the menu select `Tools` => `Debug console`)

Issue the following:

```masternode outputs```

Make note of the hash (which is your collateral_output) and index.

### Enter your Masternode details into your masternode.conf file
[From the documentchain github repo](https://github.com/Krekeler/documentchain/blob/master/doc/masternode_conf.md)

`masternode.conf` format is a space seperated text file. Each line consisting of an alias, IP address followed by port, masternode private key, collateral output transaction id and collateral output index.

```
alias ipaddress:port masternode_private_key collateral_output collateral_output_index
```

Example:

```
MN1 123.123.123.123:41319 5UFi4AmqjhYGqw1sKMfqjVvXeb57yL3aqw5VdWSG18x2xMAmags 7603c20a05258c208b58b0a0d77603b9fc93d47cfa403035f87f3ce0af814566 1
MN2 123.123.123.124:41319 5UL1E4xbdX1V5LKAe3g43PVYkYi1ksNUVXW1zamyGXQGeqHq4dk 5d898e78244f3206e0105f421cdb071d95d111a51cd88eb5511fc0dbf4bfd95f 0
```

## Update dms.conf on server

If you generated a new masternode private key, you will need to update the remote `dms.conf` files.

Shut down the daemon and then edit the file. [Example](https://documentchain.org/support/masternodes/#dmsconf)

```nano .dmscore/dms.conf```

### Edit the masternodeprivkey
If you generated a new masternode private key, you will need to update the `masternodeprivkey` value in your remote `dms.conf` file.

## Start your Masternodes

### Remote

If your remote server is not running, start your remote daemon as you normally would. 

You can confirm that remote server is on the correct block by issuing

```dms-cli getinfo```

and comparing with the official explorer at https://explorer.documentchain.org/

### Local

Finally... time to start from local.

#### Open up your Qt Wallet

From the menu select `Tools` => `Debug console`

If you want to review your `masternode.conf` setting before starting Masternodes, issue the following in the Debug console:

```masternode list-conf```

Give it the eye-ball test. If satisfied, you can start your Masternodes one of two ways.

1. `masternode start-alias [alias_from_masternode.conf]`  
Example ```masternode start-alias mn01```
2. `masternode start-many`

## Verify that Masternodes actually started

### Remote

Issue command `masternode status`
It should return you something like that:
```
dms-cli masternode status
{
    "outpoint" : "<collateral_output>-<collateral_output_index>",
    "service" : "<ipaddress>:<port>",
    "pubkey" : "<5000 DMS address>",
    "status" : "Masternode successfully started"
}
```
Command output should have "_Masternode successfully started_" in its `status` field now. If it says "_not capable_" instead, you should check your config again.
