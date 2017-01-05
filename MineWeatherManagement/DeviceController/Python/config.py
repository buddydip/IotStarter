# ===== Your specific configuration goes below / please adapt ========

# the HCP account id - trial accounts typically look like p[0-9]*trial
hcp_account_id='s0004936377trial'

# you only need to adapt this part of the URL if you are NOT ON TRIAL but e.g. on PROD
hcp_landscape_host='.hanatrial.ondemand.com'
# hcp_landscape_host='.hana.ondemand.com' # this is used on PROD

endpoint_certificate = "./hanatrial.ondemand.com.crt"
# you can download this certificate file with your browser from the app server in your landscape - it is used to check that the server is authentic
# we also provide the certificate that is valid in July 2016 at our github repo

# the following values need to be taken from the IoT Cockpit
device_id='1f0a79dd-bac3-44bd-a2d5-6069f3c38153'

# the device specific OAuth token is used as MQTT password
oauth_credentials_for_device='5657eb26a6c29fc2caec8a249cb138ae'

message_type_id_From_device='19372fe758f8b887222f_1'

# ===== nothing to be changed / configured below this line ===========
