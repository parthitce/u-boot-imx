// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <clk.h>
#include <asm/clk.h>

struct sandbox_clk_test {
	struct clk clks[SANDBOX_CLK_TEST_ID_COUNT];
	struct clk_bulk bulk;
};

static const char * const sandbox_clk_test_names[] = {
	[SANDBOX_CLK_TEST_ID_FIXED] = "fixed",
	[SANDBOX_CLK_TEST_ID_SPI] = "spi",
	[SANDBOX_CLK_TEST_ID_I2C] = "i2c",
};

int sandbox_clk_test_get(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);
	int i, ret;

	for (i = 0; i < SANDBOX_CLK_TEST_ID_COUNT; i++) {
		ret = clk_get_by_name(dev, sandbox_clk_test_names[i],
				      &sbct->clks[i]);
		if (ret)
			return ret;
	}

	return 0;
}

int sandbox_clk_test_get_by_id(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);
	struct clk *clkp, *i2c_clk;
	ulong driver_data_bkp;
	const int id = 24;
	int ret, id_bkp;

	i2c_clk = &sbct->clks[SANDBOX_CLK_TEST_ID_I2C];

	id_bkp = i2c_clk->id;
	i2c_clk->id = id;
	driver_data_bkp = i2c_clk->dev->driver_data;
	i2c_clk->dev->driver_data = (ulong)i2c_clk;

	ret = clk_get_by_id(id, &clkp);

	i2c_clk->id = id_bkp;
	i2c_clk->dev->driver_data = driver_data_bkp;

	return ret;
}

int sandbox_clk_test_get_bulk(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	return clk_get_bulk(dev, &sbct->bulk);
}

ulong sandbox_clk_test_get_parent_rate(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);
	struct clk *i2c_clk, *parent_clk;
	struct udevice *parent_bkp;
	ulong rate;

	parent_clk = &sbct->clks[SANDBOX_CLK_TEST_ID_FIXED];
	i2c_clk = &sbct->clks[SANDBOX_CLK_TEST_ID_I2C];

	parent_clk->dev->driver_data = (ulong)parent_clk;
	parent_bkp = i2c_clk->dev->parent;
	i2c_clk->dev->parent = parent_clk->dev;

	rate = clk_get_parent_rate(i2c_clk);

	i2c_clk->dev->parent = parent_bkp;
	parent_clk->dev->driver_data = 0;

	/* Check if cache'd value is correct */
	if (parent_clk->rate != 1234)
		return 0;

	return rate;
}

ulong sandbox_clk_test_get_rate(struct udevice *dev, int id)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	if (id < 0 || id >= SANDBOX_CLK_TEST_ID_COUNT)
		return -EINVAL;

	return clk_get_rate(&sbct->clks[id]);
}

ulong sandbox_clk_test_set_rate(struct udevice *dev, int id, ulong rate)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	if (id < 0 || id >= SANDBOX_CLK_TEST_ID_COUNT)
		return -EINVAL;

	return clk_set_rate(&sbct->clks[id], rate);
}

int sandbox_clk_test_enable(struct udevice *dev, int id)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	if (id < 0 || id >= SANDBOX_CLK_TEST_ID_COUNT)
		return -EINVAL;

	return clk_enable(&sbct->clks[id]);
}

int sandbox_clk_test_enable_bulk(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	return clk_enable_bulk(&sbct->bulk);
}

int sandbox_clk_test_disable(struct udevice *dev, int id)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	if (id < 0 || id >= SANDBOX_CLK_TEST_ID_COUNT)
		return -EINVAL;

	return clk_disable(&sbct->clks[id]);
}

int sandbox_clk_test_disable_bulk(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	return clk_disable_bulk(&sbct->bulk);
}

int sandbox_clk_test_free(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);
	int i, ret;

	for (i = 0; i < SANDBOX_CLK_TEST_ID_COUNT; i++) {
		ret = clk_free(&sbct->clks[i]);
		if (ret)
			return ret;
	}

	return 0;
}

int sandbox_clk_test_release_bulk(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);

	return clk_release_bulk(&sbct->bulk);
}

int sandbox_clk_test_valid(struct udevice *dev)
{
	struct sandbox_clk_test *sbct = dev_get_priv(dev);
	int i;

	for (i = 0; i < SANDBOX_CLK_TEST_ID_COUNT; i++) {
		if (!clk_valid(&sbct->clks[i]))
			return -EINVAL;
	}

	return 0;
}

static const struct udevice_id sandbox_clk_test_ids[] = {
	{ .compatible = "sandbox,clk-test" },
	{ }
};

U_BOOT_DRIVER(sandbox_clk_test) = {
	.name = "sandbox_clk_test",
	.id = UCLASS_MISC,
	.of_match = sandbox_clk_test_ids,
	.priv_auto_alloc_size = sizeof(struct sandbox_clk_test),
};
